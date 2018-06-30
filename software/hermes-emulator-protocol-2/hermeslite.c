/* 
* Copyright (C)
* 2018 - Johan Maas, PA3GSB
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Hermes Lite Emulator.
* 
* By using this emulator you have the possibility to connect to SDR programs like:
*	- linHPSDR
*	- pihpsdr
*	- Thetis
*
* SDR software packages supporting the 'HPSDR protocol-2' should be connected to the emulator.
*
*  This emulator works with the Radioberry radiocard plugged into a rpi-3.
*
*	http://www.pa3gsb.nl
*	  
*	2018 Johan PA3GSB
*
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <semaphore.h>
#include <math.h>
#include <malloc.h>
#include <signal.h>
#include <stdbool.h>
#include <linux/if_packet.h>
#include <netinet/in.h>		 
#include <netinet/if_ether.h>  
#include <netinet/ip.h>	
#include <netinet/udp.h>		
#include <netinet/tcp.h>
#include "udp.h"
#include <pigpio.h>

#define MAX_RECEIVERS 2
#define SERVICE_PORT 1024
#define MAX_DATA_SIZE 2048
#define HERMESLITE  0x06	
#define FIRMWARE_VERSION 0x28	//firmware version 4.0 (=0x28)
#define TIMEOUT_MS 100 

// port definitions from host
#define GENERAL_REGISTERS_FROM_HOST_PORT 1024
#define RECEIVER_SPECIFIC_REGISTERS_FROM_HOST_PORT 1025
#define TRANSMITTER_SPECIFIC_REGISTERS_FROM_HOST_PORT 1026
#define HIGH_PRIORITY_FROM_HOST_PORT 1027
#define AUDIO_FROM_HOST_PORT 1028
#define TX_IQ_FROM_HOST_PORT 1029

// port definitions to host
#define COMMAND_RESPONCE_TO_HOST_PORT 1024
#define HIGH_PRIORITY_TO_HOST_PORT 1025
#define MIC_LINE_TO_HOST_PORT 1026
#define WIDE_BAND_TO_HOST_PORT 1027
#define RX_IQ_TO_HOST_PORT_0 1035

int handleDiscovery(unsigned char* buffer);
int createUDPSocket(void);
void handle_packets_from_sdr_program(unsigned char* buffer,int buflen);
void *send_rx_iq_to_sdr_program(void *arg);
void rx1_spiReader(unsigned char iqdata[]);
void rx2_spiReader(unsigned char iqdata[]);
void receiver_specific_registers_from_host_port(unsigned char* buffer);
void transmitter_specific_registers_from_host_port(unsigned char* buffer);
void high_priority_from_host_port(unsigned char* buffer);
void tx_iq_from_host_port(unsigned char* buffer);
int initialize_gpio();
int handle_data_from_sdr_program();
void create_radioberry_socket();

static long rx1_sequence = 0;
static long rx2_sequence = 0;
static char sdr_client_addr[20];
static char radioberry_addr[20];

static struct sockaddr_in src_addr[8];
static struct sockaddr_in dst_addr;

int radioberry_socket=-1;
int discover_socket=-1;
int remote_port =0;

unsigned char broadcastReply[60];

static int rx1_spi_handler;
static int rx2_spi_handler;

static int running = 0;
static int nrx = 1; 
static int sampleSpeed[MAX_RECEIVERS];
static int ddc[MAX_RECEIVERS];
static int ptt = 0;
static int gain = 0;
static int dither = 1;

static int rxfreq1 = 4706000;
static int rxfreq2 = 1008000;
static int txfreq  = 3630000;

unsigned char drive_level;

sem_t mutex;

int main(int argc, char **argv) {
	
	fprintf(stderr,"\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr, "                      Radioberry V2.0 beta 2.\n");
	fprintf(stderr,	"\n");
	fprintf(stderr, "                 Emulator Protocol-2 version 30-6-2018 \n");
	fprintf(stderr,	"\n");
	fprintf(stderr,	"                       !!!Under construction!!!\n");
	fprintf(stderr,	"\n");
	fprintf(stderr,	"\n");
	fprintf(stderr, "                      Have fune Johan PA3GSB\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "====================================================================\n");
	fprintf(stderr, "====================================================================\n");
	
	sem_init(&mutex, 0, 1);	
	
	initialize_gpio();
	
	fprintf(stderr, "\n\nhermeslite protocol-2 emulator starts .... \n\n");

	pthread_t pid1; 
	pthread_create(&pid1, NULL, send_rx_iq_to_sdr_program, NULL);
	
	handle_data_from_sdr_program();
	
	fprintf(stderr, "hermeslite protocol-2 emulator stopped. \n");
}

int handle_data_from_sdr_program() {	
	int emulator_socket = -1;
	int saddr_len,buflen;
	struct sockaddr_in saddr;
	unsigned char* buffer = (unsigned char *)malloc(65536); 
	memset(buffer,0,65536);
	
	emulator_socket=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL)); 
	if(emulator_socket<0)
	{
		printf("error in socket\n");
		return -1;
	}
	while(1)
	{
		saddr_len=sizeof saddr;
		buflen=recvfrom(emulator_socket,buffer,65536,0,(struct sockaddr *) &saddr,(socklen_t *)&saddr_len);
		if(buflen<0)
		{
			perror("hermeslite protocol-2 emulator stops unexpectedly");
			return -1;
		}
		handle_packets_from_sdr_program(buffer,buflen);
	}
	close(emulator_socket);	
}	

int initialize_gpio() {
	if (gpioInitialise() < 0) {
		perror("hpsdr protocol-2: gpio could not be initialized. \n");
		exit(-1);
	}
	gpioSetMode(13, PI_INPUT); 	//rx1 samples
	gpioSetMode(16, PI_INPUT);	//rx2 samples
	gpioSetMode(20, PI_INPUT); 
	gpioSetMode(21, PI_OUTPUT); 
	
	rx1_spi_handler = spiOpen(0, 15625000, 49155);  //channel 0
	if (rx1_spi_handler < 0) {
		perror("radioberry_protocol: spi bus rx1 could not be initialized. \n");
		exit(-1);
	}
	rx2_spi_handler = spiOpen(1, 15625000, 49155); 	//channel 1
	if (rx2_spi_handler < 0) {
		perror("radioberry_protocol: spi bus rx2 could not be initialized. \n");
		exit(-1);
	}
	return 0;
}

void *send_rx_iq_to_sdr_program(void *arg) {
	unsigned char* iqbuffer_rx1 = (unsigned char *)malloc(2048); 
	memset(iqbuffer_rx1,0,2048);
	unsigned char* iqbuffer_rx2 = (unsigned char *)malloc(2048); 
	memset(iqbuffer_rx2,0,2048);
	unsigned char iqdata[6];
	unsigned int index_rx1 = 16;
	unsigned int index_rx2 = 16;

	iqbuffer_rx1[12] = 0x00; 	iqbuffer_rx2[12] = 0x00;	//MSB bitspersample
	iqbuffer_rx1[13] = 0x18; 	iqbuffer_rx2[13] = 0x18; 	//LSB bitspersample 24 bits / sample.
	iqbuffer_rx1[14] = 0x00; 	iqbuffer_rx2[14] = 0x00;	//MSB samplesperframe
	iqbuffer_rx1[15] = 0xEE; 	iqbuffer_rx2[15] = 0xEE;	//LSB samplesperframe 144-16 / 6  = 238 IQ samples
	
	while(1) {
		if (running) {
			while (ptt) {usleep(20000);}
	
			sem_wait(&mutex); 
			
			gpioWrite(21, 0); 	// ptt off
			
			if (gpioRead(13) == 1) {	
				int i =0;
				for (i; i< 64; i++){
					rx1_spiReader(iqdata);
					int j =0;
					for (j; j< 6; j++){
						iqbuffer_rx1[index_rx1++] = iqdata[j];
					}
					if (index_rx1 >= 1444) {
						iqbuffer_rx1[0]=rx1_sequence>>24;
						iqbuffer_rx1[1]=rx1_sequence>>16;
						iqbuffer_rx1[2]=rx1_sequence>>8;
						iqbuffer_rx1[3]=rx1_sequence;

						send_udp_packet(radioberry_socket, src_addr[ddc[0]], dst_addr, iqbuffer_rx1, 1444);

						index_rx1 = 16;
						rx1_sequence++;		
					}
				}
			}
			if (nrx == 2) {
				if (gpioRead(16) == 1) {
					int i =0;
					for (i; i< 64; i++){
						rx2_spiReader(iqdata);
						int j =0;
						for (j; j< 6; j++){
							iqbuffer_rx2[index_rx2++] = iqdata[j];
						}
					}
					if (index_rx2 >= 1444) {
						iqbuffer_rx2[0]=rx2_sequence>>24;
						iqbuffer_rx2[1]=rx2_sequence>>16;
						iqbuffer_rx2[2]=rx2_sequence>>8;
						iqbuffer_rx2[3]=rx2_sequence;
						
						send_udp_packet(radioberry_socket, src_addr[ddc[1]], dst_addr, iqbuffer_rx2, 1444);
						
						index_rx2 = 16;
						rx2_sequence++;
					}
				}
			}
			sem_post(&mutex);
		} else {usleep(20000);}
	}
}

void rx1_spiReader(unsigned char iqdata[]) {	
	iqdata[0] = (sampleSpeed[0] & 0x03);
	iqdata[1] = (~(gain & 0x2F));
	iqdata[2] = ((rxfreq1 >> 24) & 0xFF);
	iqdata[3] = ((rxfreq1 >> 16) & 0xFF);
	iqdata[4] = ((rxfreq1 >> 8) & 0xFF);
	iqdata[5] = (rxfreq1 & 0xFF);
			
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
}

void rx2_spiReader(unsigned char iqdata[]) {	
	iqdata[0] = (sampleSpeed[0] & 0x03);
	iqdata[1] = (~(gain & 0x2F));
	iqdata[2] = ((rxfreq2 >> 24) & 0xFF);
	iqdata[3] = ((rxfreq2 >> 16) & 0xFF);
	iqdata[4] = ((rxfreq2 >> 8) & 0xFF);
	iqdata[5] = (rxfreq2 & 0xFF);
			
	spiXfer(rx2_spi_handler, iqdata, iqdata, 6);
}

void handle_packets_from_sdr_program(unsigned char* buffer,int buflen) {
	int iphdrlen;
	struct iphdr *ip = (struct iphdr*)(buffer + sizeof (struct ethhdr));
	
	if (ip->protocol == 17) {
		iphdrlen =ip->ihl*4;
		struct udphdr *udp = (struct udphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));
		unsigned char * data = (buffer + iphdrlen  + sizeof(struct ethhdr) + sizeof(struct udphdr));
		int remaining_data = buflen - (iphdrlen  + sizeof(struct ethhdr) + sizeof(struct udphdr));
		
		switch (ntohs(udp->dest))  {
			case GENERAL_REGISTERS_FROM_HOST_PORT:
				if (radioberry_socket == -1) {
					struct sockaddr_in remaddr;	
					memset((char *)&remaddr, 0, sizeof(remaddr));
					remaddr.sin_addr.s_addr = ip->daddr;  
					strcpy(radioberry_addr, inet_ntoa(remaddr.sin_addr));
					remote_port = udp->source;
				}
				if (data[4] == 2) { 
					handleDiscovery(buffer);	
				} //else if (data[0] != 0xEF & data[1] != 0xFE && data[2] != 0x02 && data[4] == 0) {
					//fprintf(stderr,"GENERAL PACKET FROM HOST --- which info is important?\n");}
				break;
			case RECEIVER_SPECIFIC_REGISTERS_FROM_HOST_PORT:
				receiver_specific_registers_from_host_port(data);
				break;	
			case TRANSMITTER_SPECIFIC_REGISTERS_FROM_HOST_PORT:
				transmitter_specific_registers_from_host_port(data);
				break;
			case HIGH_PRIORITY_FROM_HOST_PORT:
				high_priority_from_host_port(data);
				break;
			case AUDIO_FROM_HOST_PORT:
				// no implementation
				break;
			case TX_IQ_FROM_HOST_PORT:
				tx_iq_from_host_port(data);
				break;
			default:
				break;
		}
	}
}

void receiver_specific_registers_from_host_port(unsigned char* data) {
	// the radioberry suports 2 ddc.
	int i = 0, ddc_no = 0, speed = 0, lnrx = 0;
	for (i=0; i <8; i++){
		if ( (data[7] >> i) & 1){
			ddc[ddc_no++] = i; lnrx++; 
			//fprintf(stderr, "ddc no %d \n", i); 
		}
	} 
	//fprintf(stderr,"Numer of receivers: %d \n", lnrx);
	speed = data[18+(ddc[0]*6)] << 8 | data[19+(ddc[0]*6)];	
	sampleSpeed[0] = speed/48==1?0x00:speed/48==2?0x01:speed/48==4?0x02:speed/48==8?0x03:0x04;
	//fprintf(stderr,"sample speed receiver[0] =  %d \n", sampleSpeed[0]);
	if (lnrx == 2){
		speed = data[18+(ddc[1]*6)] << 8 | data[19+(ddc[1]*6)];
		sampleSpeed[1] = speed/48==1?0x00:speed/48==2?0x01:speed/48==4?0x02:speed/48==8?0x03:0x04;
		//fprintf(stderr,"sample speed receiver[1] =  %d \n", sampleSpeed[1]);	
	}
	nrx = lnrx;
}

void transmitter_specific_registers_from_host_port(unsigned char* buffer) {
			//1026 todo; things like cw handling...
}

void high_priority_from_host_port(unsigned char* data) {
			
	create_radioberry_socket();
	
	running = ((data[4] & 0x01) == 0x01)? 1:0;
	//fprintf(stderr," running %.2X %d \n",data[4], running);
	ptt = ((data[4] & 0x02) == 0x02) ? 1:0; 
	//fprintf(stderr," ptt %d ",ptt);
	//data[5] [0] = CWX, [1] = Dot, [2] = Dash CW TODO!!
	int rxphase1, rxphase2, txphase;
	//rx 
	rxphase1 = data[9+(ddc[0]*4)] << 24 | data[10+(ddc[0]*4)] << 16  | data[11+(ddc[0]*4)] << 8 | data[12+(ddc[0]*4)] ;
	rxphase2 = data[9+(ddc[1]*4)] << 24 | data[10+(ddc[1]*4)] << 16  | data[11+(ddc[1]*4)] << 8 | data[12+(ddc[1]*4)] ;
	rxfreq1 =  (long) (((double) rxphase1 * 122880000.0) / 4294967296.0);
	//fprintf(stderr,"rxfreq1 %d \n", rxfreq1);
	rxfreq2 =  (long) (((double) rxphase2 * 122880000.0) / 4294967296.0);
	//fprintf(stderr,"rxfreq2 %d \n", rxfreq2);
	//tx
	txphase = ( data[329] << 24 | data[330] << 16  | data[331] << 8 | data[332] );
	txfreq =  (long) (((double) txphase * 122880000.0) / 4294967296.0);
	//fprintf(stderr,"txfreq %d \n", txfreq);
	drive_level = data[345] & 0xFF;
	//fprintf(stderr,"drive_level %d \n", drive_level);
	gain = ((int) ((signed char) data[1443])) + 12;
	//fprintf(stderr, "ad9866 gain = %d rx-gain = %d \n", gain, (gain -12));
}

void create_radioberry_socket() {
	if (radioberry_socket == -1) {	
		int i = 0;
		for(i; i < 8; i++) {	
			src_addr[i].sin_family = PF_INET;
			src_addr[i].sin_port = htons(RX_IQ_TO_HOST_PORT_0 + i); 
			inet_aton(radioberry_addr, &src_addr[i].sin_addr);
		}
		dst_addr.sin_family = PF_INET;
		dst_addr.sin_port = htons(ntohs(remote_port));
		inet_aton(sdr_client_addr, &dst_addr.sin_addr);	
		if((radioberry_socket = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0){
			perror("socket");
			exit(-1);
		}
		int optval = 1;
		if (setsockopt(radioberry_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
			fprintf (stderr, "Warning: no reuse\n");
		}
		if (setsockopt(radioberry_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
			fprintf (stderr, "Warning: no port reuse\n");
		}	
	}
}

void tx_iq_from_host_port(unsigned char* buffer) {
	
	// protocol-2 the rate is 192ksps  (radioberry uses 48K; downsampling required!)
	// i and q (msb first using 24b/sample, radioberry uses only 16b/sample; using 2 msb bytes.) from index 4 to 1443 (240 i and q samples)
	// under construction....
	if (!ptt) return;
	
	sem_wait(&mutex);
	
	gpioWrite(21, 1); ;	// ptt on
	
	//set the tx freq; we are doing this per block of data.
	tx_iqdata[0] = 0x00;
	tx_iqdata[1] = 0x00;
	tx_iqdata[2] = ((txfreq >> 24) & 0xFF);
	tx_iqdata[3] = ((txfreq >> 16) & 0xFF);
	tx_iqdata[4] = ((txfreq >> 8) & 0xFF);
	tx_iqdata[5] = (txfreq & 0xFF);				
	spiXfer(rx2_spi_handler, tx_iqdata, tx_iqdata, 6);
	
	int index = 4;	
	while (index =< 1444) {
		gpioRead(20) == 1) {};	// wait if TX buffer is full.	

		tx_iqdata[0] = 0;
		tx_iqdata[1] = drive_level / 6.4;  // convert drive level from 0-255 to 0-39 )
		tx_iqdata[2] = ((buffer[index++]) & 0xFF); 
		tx_iqdata[3] = ((buffer[index++]) & 0xFF); 
		tx_iqdata[4] = ((buffer[++index]) & 0xFF); 
		tx_iqdata[5] = ((buffer[++index]) & 0xFF); 
		spiXfer(rx1_spi_handler, tx_iqdata, tx_iqdata, 6);
		index = index + 19;
	}
	sem_post(&mutex); 
}

int createUDPSocket() {
	struct sockaddr_in myaddr;
	if ((discover_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("cannot create discover socket\n");
		return -1;
	}
	struct timeval timeout;      
    timeout.tv_sec = 0;
    timeout.tv_usec = TIMEOUT_MS;

	if (setsockopt(discover_socket, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0) {
		perror("setsockopt failed\n");
		return -1;
	}
	int optval = 1;
    setsockopt(discover_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    setsockopt(discover_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
		
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = PF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	myaddr.sin_port = htons(SERVICE_PORT);

	if (bind(discover_socket, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind to discover socket failed");
		return -1;
	}
	return 0;
}

void fillDiscoveryReplyMessage() {
	memset(broadcastReply, 0, sizeof(broadcastReply));
	int i = 5;
	for (i; i < 11; i++) broadcastReply[i] = i - 5; //MAC 00-01-02-03-04-05
	broadcastReply[4]  = 0x02;
	broadcastReply[11] = HERMESLITE;	
	broadcastReply[13] = FIRMWARE_VERSION;				
}

int handleDiscovery(unsigned char* buffer) {
	int iphdrlen;
	struct sockaddr_in remaddr;	
	socklen_t addrlen = sizeof(remaddr);
	if (running ==  0) {
		if (radioberry_socket != -1) close(radioberry_socket);
		radioberry_socket = -1;
	} else return 0;
	
	struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));
	iphdrlen =ip->ihl*4;
	struct udphdr *udp = (struct udphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));   
	struct sockaddr_in source,dest;	 
	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = ip->saddr;     
	memset(&dest, 0, sizeof(dest));
	dest.sin_addr.s_addr = ip->daddr;   
	memset((char *)&remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = PF_INET;
	remaddr.sin_addr.s_addr = ip->saddr;  
	remaddr.sin_port = udp->source;
	
	fprintf(stderr,"Discovery packet received \n");
	fprintf(stderr,"SDR Program IP-address %s  \n", inet_ntoa(remaddr.sin_addr)); 
	fprintf(stderr,"Discovery Port %d \n", ntohs(remaddr.sin_port));

	strcpy(sdr_client_addr, inet_ntoa(remaddr.sin_addr));	
	fillDiscoveryReplyMessage();
	createUDPSocket();			
	if (sendto(discover_socket, broadcastReply, sizeof(broadcastReply), 0, (struct sockaddr *)&remaddr, addrlen) < 0) {
		perror("Discovery reply sendto error");
		return -1;
	}
	close(discover_socket);
	return 0;
}
// end of source.