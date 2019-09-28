/* 
* Copyright (C)
* 2017, 2018, 2019 - Johan Maas, PA3GSB
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
* This programs is the c part of the firmware for the RadioBerry running with ethernet (eth8720) module. 
*
* By using this program you have the possibility to connect to SDR programs like:
*	- pihpsdr
*	- Quisk
*
*	Using the 'old HPSDR protocol'; also called protocol-1
*
*  This emulator works with the Radioberry radiocard.
*
*	http://www.pa3gsb.nl
*	  
*	Johan PA3GSB
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <math.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define __USE_GNU
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <ifaddrs.h>
#include <sys/select.h>

#include <pigpio.h>

//#include "radioberry.h"

int rx1_phase = 0;
int rx2_phase = 0;
int tx_phase = 0;


void sendPacket(void);
void handlePacket(char* buffer);
void processPacket(char* buffer);
void fillPacketToSend(void);
void printIntroScreen(void);
void *packetreader(void *arg);
void *spiReader(void *arg);
void *spiWriter(void *arg);
void put_tx_buffer(unsigned char  value);
unsigned char get_tx_buffer(void);
void getRadioberryInformation(void);
void getRunningRadioberryInformation(void);
void startRadioberryRadio(void);
void stopRadioberryRadio(void);
int createRadioberrySocket(void);
void  receiveudp(void);
void spi_control_rx1_phase(void);

							
int sock_Radioberry = -1;
int sock_TCP_Server = -1;
int sock_TCP_Client = -1;

int udp_retries=0;
int active = 0;

#define RX_MAX 10
#define TX_MAX 4800 
#define TX_MAX_BUFFER (TX_MAX * 4)
unsigned char tx_buffer[TX_MAX_BUFFER];
int fill_tx = 0; 
int use_tx  = 0;
unsigned char drive_level;
unsigned char prev_drive_level;
int MOX = 0;
int rbMOX = -1;
sem_t rx_empty;
sem_t rx_full;
sem_t tx_empty;
sem_t tx_full;
sem_t mutex;
sem_t radiosync;

int tx_count =0;

unsigned char rx_buffer[RX_MAX][504];
int rx_read_index = 0;

void rx1_spiReader(unsigned char iqdata[]);
void rx2_spiReader(unsigned char iqdata[]);

static int rx1_spi_handler;
static int rx2_spi_handler;

unsigned char iqdata[6 * 64];
unsigned char tx_iqdata[6];

#define SERVICE_PORT	1024

int nrx = 1; // n Receivers

int freq = 4706000;
int freq2 = 1008000;
int txfreq = 3630000;

int att = 0;
int dither = 0;
int rando = 0;
int sampleSpeed = 0;

#define SYNC 0x7F
int last_sequence_number = 0;
uint32_t last_seqnum=0xffffffff, seqnum; 
uint32_t rb_last_seqnum=0xffffffff, rb_seqnum; 

#define FIRMWARE_VERSION 0x2A
unsigned char hpsdrdata[1032];
uint8_t header_hpsdrdata[4] = { 0xef, 0xfe, 1, 6 };
uint8_t sync_hpsdrdata[8] = { SYNC, SYNC, SYNC, 0, 0, 0, 0, FIRMWARE_VERSION};
	
unsigned char broadcastReply[60];
#define TIMEOUT_MS      1000     

int running = 0;
int fd;									/* our socket */


struct sockaddr_in rbaddr;
struct sockaddr_in myaddr;				/* our address */
struct sockaddr_in remaddr;				/* remote address */

socklen_t addrlen = sizeof(remaddr);	/* length of addresses */
int recvlen;							/* # bytes received */

struct timeval t20;
struct timeval t21;
float elapsed;

#define MAX11613_ADDRESS	0x34
unsigned char data[8];
unsigned int i2c_bus = 1;
int i2c_handler = 0;
int vswr_active = 0;

#define ADDR_ALEX 			0x21 		/* PCA9555 address 1 */
int i2c_alex_handler = 0;
int i2c_alex = 0;
int alex_manual = 0;
uint16_t i2c_alex_data = 0;
uint16_t i2c_data = 0;


int radioberry_initialized = 0;
static char radioberry_addr[20];

void signal_shutdown(int signum);


float timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

void initVSWR(){
	unsigned char config[1];
	config[0] = 0x07;
	if (i2cWriteDevice(i2c_handler, config, 1) == 0 ) {
		vswr_active = 1;
	}	
}

void initALEX(){
	int result = 0;
	unsigned char data[3];

	/* configure all pins as output */
	data[0] = 0x06;
	data[1] = 0x00;
	data[2] = 0x00;
	result = i2cWriteDevice(i2c_alex_handler, data, 3);
	
	if (result >= 0) {
		data[0] = 0x02;
		data[1] = 0x00;
		data[2] = 0x00;
		/* set all pins to low */
		result = i2cWriteDevice(i2c_alex_handler, data, 3);
	}
	
	if (result >= 0) {
		i2c_alex = 1;
		fprintf(stderr, "alex interface found and initialized \n");
	} else fprintf(stderr, "no alex interface found\n");
}

int main(int argc, char **argv)
{
	int yes = 1;

	printIntroScreen();	
	
	sem_init(&mutex, 0, 1);	
	sem_init(&tx_empty, 0, TX_MAX); 
    sem_init(&tx_full, 0, 0); 
	sem_init(&rx_empty, 0, RX_MAX); 
    sem_init(&rx_full, 0, 0);   
    sem_init(&radiosync, 0, 0);   

	unsigned int rev = gpioHardwareRevision();  
	//A03111 hex
	int rpi4 = (rev >= 10498321) ? 1 : 0;
	if (rpi4) fprintf(stderr, "radioberry running at rpi-4\n");
	
	if (gpioInitialise() < 0) {
		fprintf(stderr,"hpsdr_protocol (original) : gpio could not be initialized. \n");
		exit(-1);
	}
	
	gpioSetMode(21, PI_OUTPUT); 
		
	i2c_handler = i2cOpen(i2c_bus, MAX11613_ADDRESS, 0);
	if (i2c_handler >= 0)  initVSWR();
	i2c_alex_handler = i2cOpen(i2c_bus, ADDR_ALEX, 0);
	if (i2c_alex_handler >= 0)  initALEX();
	if (rpi4) 
		rx1_spi_handler = spiOpen(0, 9000000, 49155);  //channel 0
	else 
		rx1_spi_handler = spiOpen(0, 15625000, 49155);  //channel 0
	
	if (rx1_spi_handler < 0) {
		fprintf(stderr,"radioberry_protocol: spi bus rx1 could not be initialized. \n");
		exit(-1);
	}
	if (rpi4) 
		rx2_spi_handler = spiOpen(1, 9000000, 49155); 	//channel 1 
	else
		rx2_spi_handler = spiOpen(1, 15625000, 49155); 	//channel 1 
		
	if (rx2_spi_handler < 0) {
		fprintf(stderr,"radioberry_protocol: spi bus rx2 could not be initialized. \n");
		exit(-1);
	}	
	
	fprintf(stderr, "Radioberry radio, running initialisation... \n");
	while(radioberry_initialized == 0){
		fprintf(stderr, ".");
		getRadioberryInformation();
		usleep(1000000);
	}
	fprintf(stderr, "\n");
	
	createRadioberrySocket();
	
	spi_control_rx1_phase();
	
	startRadioberryRadio();
	
	/*
	receiveudp();
	
	if (rx1_spi_handler !=0)
		spiClose(rx1_spi_handler);
	if (rx2_spi_handler !=0)
		spiClose(rx2_spi_handler);
	
	
		
	gpioTerminate();
	
	return 0;
	*/
	
	pthread_t pid1, pid2, pid3, pid4; 
	pthread_create(&pid2, NULL, packetreader, NULL); 
	pthread_create(&pid3, NULL, spiWriter, NULL);
	pthread_create(&pid4, NULL, spiReader, NULL);

	/* create a UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return -1;
	}
	struct timeval timeout;      
    timeout.tv_sec = 0;
    timeout.tv_usec = TIMEOUT_MS;

	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0)
		perror("setsockopt failed\n");
		
	/* bind the socket to any valid IP address and a specific port */
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(SERVICE_PORT);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return -1;
	}
	if ((sock_TCP_Server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket tcp");
		return -1;
	}
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_REUSEADDR, (void *)&yes, sizeof(yes));

	int tcpmaxseg = 1032;
	setsockopt(sock_TCP_Server, IPPROTO_TCP, TCP_MAXSEG, (const char *)&tcpmaxseg, sizeof(int));
	int sndbufsize = 65535;
	int rcvbufsize = 65535;
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_SNDBUF, (const char *)&sndbufsize, sizeof(int));
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVBUF, (const char *)&rcvbufsize, sizeof(int));
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeout)); //added

	if (bind(sock_TCP_Server, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
	{
		perror("bind tcp");
		return -1;
	}
	listen(sock_TCP_Server, 1024);
	int flags = fcntl(sock_TCP_Server, F_GETFL, 0);
    fcntl(sock_TCP_Server, F_SETFL, flags | O_NONBLOCK);

	while(1) {if (running) {active = 1; sendPacket();} else {active = 0; usleep(20000);}}
	
	if (rx1_spi_handler !=0)
		spiClose(rx1_spi_handler);
	if (rx2_spi_handler !=0)
		spiClose(rx2_spi_handler);
	
	if (sock_TCP_Client >= 0)
	{
		close(sock_TCP_Client);
	}

	if (sock_TCP_Server >= 0)
	{
		close(sock_TCP_Server);
	}
		
	gpioTerminate();
}

void *packetreader(void *arg) {
	int size, bytes_read, bytes_left;
	unsigned char buffer[2048];
	uint32_t *code0 = (uint32_t *) buffer; 
	
	while(1) {
		
		if (sock_TCP_Client >= 0) {
			// handle TCP protocol.
			bytes_read=0;
			bytes_left=1032;
			while (bytes_left > 0) {
				size = recvfrom(sock_TCP_Client, buffer+bytes_read, (size_t) bytes_left, 0, NULL, 0);
				if (size < 0 && errno == EAGAIN) continue;
				if (size < 0) break;
				bytes_read += size;
				bytes_left -= size;
			}
			if (bytes_read == 1032) handlePacket(buffer); else fprintf(stderr, "tcp packet received; wrong length %d \n", bytes_read);
		} 
		else {
			// handle UDP protocol.
			recvlen = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&remaddr, &addrlen);
			if (recvlen > 0) { udp_retries = 0; handlePacket(buffer); } else udp_retries++;
			// If nothing has arrived via UDP for some time (defined by socket timeout * 10) , try to open TCP connection.
			if (sock_TCP_Client < 0 && udp_retries > 10)
			{
				if((sock_TCP_Client = accept(sock_TCP_Server, NULL, NULL)) > -1)
				{
						fprintf(stderr, "sock_TCP_Client: %d connected to sock_TCP_Server: %d\n", sock_TCP_Client, sock_TCP_Server);
				}
				udp_retries=0;
			}
		}
	}
}

void handlePacket(char* buffer){
	uint32_t code;
	memcpy(&code, buffer, 4);
	switch (code)
	{
		default:
			fprintf(stderr, "Received packages not for me! \n");
			break;
		case 0x0002feef:
			fprintf(stderr, "Discovery packet received \n");
			fprintf(stderr,"SDR Program IP-address %s  \n", inet_ntoa(remaddr.sin_addr)); 
			fprintf(stderr, "Discovery Port %d \n", ntohs(remaddr.sin_port));
			memset(broadcastReply, 0, 60);
			unsigned char reply[14] = {0xEF, 0xFE, 0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, FIRMWARE_VERSION, 6, 'T', 'C', 'P' };
			memcpy(broadcastReply, reply, 14);
			if (sock_TCP_Client > -1) {
				send(sock_TCP_Client, broadcastReply, 60, 0);
				close(sock_TCP_Client);
				sock_TCP_Client = -1;
				}
			else if (sendto(fd, broadcastReply, sizeof(broadcastReply), 0, (struct sockaddr *)&remaddr, addrlen) < 0) fprintf(stderr, "broadcast reply error");
			break;
		case 0x0004feef:
			fprintf(stderr, "SDR Program sends Stop command \n");
			running = 0;
			while (active) usleep(1000);
			stopRadioberryRadio();
			last_sequence_number = 0;
			if (sock_TCP_Client > -1)
			{
				close(sock_TCP_Client);
				sock_TCP_Client = -1;
				fprintf(stderr, "SDR Program sends TCP Stop command \n");
			} else fprintf(stderr, "SDR Program sends UDP Stop command \n"); 
			break;
		case 0x0104feef:
		case 0x0204feef:
		case 0x0304feef:
		case 0x1104feef: 
			startRadioberryRadio();
			usleep(1000);
			running = 1;
			if (sock_TCP_Client > -1)
				fprintf(stderr, "SDR Program sends TCP Start command \n");
			else
				fprintf(stderr, "SDR Program sends UDP Start command \n");
			break;
		case 0x0201feef:
			processPacket(buffer);
			break;		
	}
}	

void handleALEX(char* buffer)
{
	if (i2c_alex & (buffer[523] & 0xFE) == 0x12) {
		alex_manual = ((buffer[525] & 0x40) == 0x40) ? 1: 0;
		if (alex_manual) {
			i2c_alex_data = ((buffer[526] & 0x8F) << 8 ) | (buffer[527] & 0xFF);
		} else {
			//firmware does determine the filter.
			uint16_t hpf = 0, lpf = 0;
			
			if(freq < 1416000) hpf = 0x20; /* bypass */
			else if(freq < 6500000) hpf = 0x10; /* 1.5 MHz HPF */
			else if(freq < 9500000) hpf = 0x08; /* 6.5 MHz HPF */
			else if(freq < 13000000) hpf = 0x04; /* 9.5 MHz HPF */
			else if(freq < 20000000) hpf = 0x01; /* 13 MHz HPF */
			else hpf = 0x02; /* 20 MHz HPF */
			
			if(freq > 32000000) lpf = 0x10; /* bypass */
			else if(freq > 22000000) lpf = 0x20; /* 12/10 meters */
			else if(freq > 15000000) lpf = 0x40; /* 17/15 meters */
			else if(freq > 8000000) lpf = 0x01; /* 30/20 meters */
			else if(freq > 4500000) lpf = 0x02; /* 60/40 meters */
			else if(freq > 2400000) lpf = 0x04; /* 80 meters */
			else lpf = 0x08; /* 160 meters */
			
			i2c_alex_data = hpf << 8 | lpf;
		}
	}
	if(i2c_alex)
	{
		if(i2c_data != i2c_alex_data)
		{
			fprintf(stderr, "Set Alex data to output = %x \n", i2c_alex_data);
			i2c_data = i2c_alex_data;
			unsigned char ldata[3];
			ldata[0] = 0x02;
			ldata[1] = ((i2c_alex_data >> 8) & 0xFF);
			ldata[2] = (i2c_alex_data & 0xFF);
			i2cWriteDevice(i2c_alex_handler, ldata, 3);
		}
	}
}

int change = 0;
#define assign_change(a,b,c) if ((a) != b) { change = 1; b = (a); fprintf(stderr, "%20s= %08lx (%10ld)\n", c, (long) b, (long) b ); }

int determine_freq(int base_index, char* buffer) {
	return ( ((buffer[ base_index + 1] & 0xFF) << 24) + ((buffer[ base_index + 2] & 0xFF) << 16) + ((buffer[base_index + 3] & 0xFF) << 8) + (buffer[base_index + 4] & 0xFF) );
}

void processPacket(char* buffer)
{
	seqnum=((buffer[4]&0xFF)<<24)+((buffer[5]&0xFF)<<16)+((buffer[6]&0xFF)<<8)+(buffer[7]&0xFF);
	if (seqnum != last_seqnum + 1) {
	  fprintf(stderr,"Radioberry firmware SEQ ERROR: last %ld, recvd %ld\n", (long) last_seqnum, (long) seqnum);
	}
	last_seqnum = seqnum;
	MOX = ((buffer[11] & 0x01)==0x01) ? 1:0;
	if ((buffer[ 11] & 0xFE) == 0x14) assign_change((buffer[ 15] & 0x1F), att, "Attenuator");
	if ((buffer[523] & 0xFE) == 0x14) assign_change((buffer[527] & 0x1F), att, "Attenuator");
	
	if ((buffer[ 11] & 0xFE) == 0x00) {
		assign_change((((buffer[ 15] & 0x38) >> 3) + 1), nrx, "Receivers");
		assign_change((buffer[12] & 0x03), sampleSpeed, "SampleRate"); 
		if ((buffer[14] & 0x08) == 0x08) {assign_change(1, dither, "Dither");} else assign_change(0, dither, "Dither");
		if ((buffer[14] & 0x10) == 0x10) {assign_change(1, rando,  "Random");} else assign_change(0, rando,  "Random");
	}
	
	if ((buffer[523] & 0xFE)  == 0x00) {
		assign_change((((buffer[527] & 0x38) >> 3) + 1), nrx, "Receivers");
		if ((buffer[526] & 0x08) == 0x08) {assign_change(1, dither, "Dither");} else assign_change(0, dither, "Dither");
		if ((buffer[526] & 0x10) == 0x10) {assign_change(1, rando,  "Random");} else assign_change(0, rando,  "Random");
	}
	if ((buffer[ 11] & 0xFE) == 0x02) assign_change(determine_freq( 11, buffer), txfreq, "TX FREQ");
	if ((buffer[523] & 0xFE) == 0x02) assign_change(determine_freq(523, buffer), txfreq, "TX FREQ");
	if ((buffer[ 11] & 0xFE) == 0x04) assign_change(determine_freq( 11, buffer),   freq, "RX-1 FREQ");
	if ((buffer[523] & 0xFE) == 0x04) assign_change(determine_freq(523, buffer),   freq, "RX-1 FREQ");
	if ((buffer[ 11] & 0xFE) == 0x06) assign_change(determine_freq( 11, buffer),  freq2, "RX-2 FREQ");
	if ((buffer[523] & 0xFE) == 0x06) assign_change(determine_freq(523, buffer),  freq2, "RX-2 FREQ");
	
	if ((buffer[523] & 0xFE) == 0x12) assign_change(buffer[524], drive_level, "Drive Level"); 
	
	handleALEX(buffer);
	
	if (change) spi_control_rx1_phase();
	change = 0;
 
	int frame = 0;
	for (frame; frame < 2; frame++)
	{
		int coarse_pointer = frame * 512 + 8;
		int j = 8;
		for (j; j < 512; j += 8)
		{
			int k = coarse_pointer + j;
			if (rbMOX) {
				sem_wait(&tx_empty);
				int i = 0;
				for (i; i < 4; i++){
					put_tx_buffer(buffer[k + 4 + i]);	
				}
				sem_post(&tx_full);
				
				sem_post(&radiosync);
			}
		}
	}
}

void sendPacket() {
	fillPacketToSend();
	if (sock_TCP_Client > -1) {
		if (send(sock_TCP_Client, hpsdrdata, sizeof(hpsdrdata), 0) != 1032) fprintf(stderr, "TCP send error\n");
	} else {
		if (sendto(fd, hpsdrdata, sizeof(hpsdrdata), 0, (struct sockaddr *)&remaddr, addrlen) != 1032) fprintf(stderr, "UDP send error\n");
	}
}

void fillPacketToSend() {
		memset(hpsdrdata,0,1032);
		memcpy(hpsdrdata, header_hpsdrdata, 4);
		hpsdrdata[4] = ((last_sequence_number >> 24) & 0xFF);
		hpsdrdata[5] = ((last_sequence_number >> 16) & 0xFF);
		hpsdrdata[6] = ((last_sequence_number >> 8) & 0xFF);
		hpsdrdata[7] = (last_sequence_number & 0xFF);
		last_sequence_number++;
		
		memcpy(hpsdrdata + 8, sync_hpsdrdata, 8);
		memcpy(hpsdrdata + 520, sync_hpsdrdata, 8);

		int frame = 0;
		for (frame; frame < 2; frame++) {
			int coarse_pointer = frame * 512; // 512 bytes total in each frame
			
			if (!rbMOX) {
				sem_wait(&rx_full);
				memcpy(hpsdrdata + coarse_pointer + 16, rx_buffer[rx_read_index], 504);
				rx_read_index = (rx_read_index + 1) % RX_MAX; 
				sem_post(&rx_empty);
			} 
			else
			{
				if (vswr_active) {
					if (frame == 0) {
						//reading once per 2 frames!
						int result = i2cReadDevice(i2c_handler, data, 8);
						hpsdrdata[11 + coarse_pointer] = 0x08;
						hpsdrdata[14 + coarse_pointer] = (data[6] & 0x0F); 
						hpsdrdata[15 + coarse_pointer] = data[7];
					}
					if (frame == 1) {
						hpsdrdata[11 + coarse_pointer] = 0x10;
						hpsdrdata[12 + coarse_pointer] = (data[0] & 0x0F); 
						hpsdrdata[13 + coarse_pointer] = data[1];
					}
				}

				sem_wait(&radiosync);
			}
		}
}



void *spiReader(void *arg) {
	
	int pointer = 0;
	int rx_fill_index = 0;
	int lnrx = nrx;
	
	while(1) {
		
		sem_wait(&rx_empty);
		
		//ptt off
		if (!MOX && rbMOX!=MOX) {gpioWrite(21, 0); rbMOX = MOX;}
		
		unsigned char buffer[2048];
		struct sockaddr_in remaddr;	
		int recvlen = recvfrom(sock_Radioberry, buffer, sizeof(buffer), 0, (struct sockaddr *)&remaddr, &addrlen);
		
		if (recvlen == 1030) {
			rb_seqnum=((buffer[0]&0xFF)<<24)+((buffer[1]&0xFF)<<16)+((buffer[2]&0xFF)<<8)+(buffer[3]&0xFF);
			
			if (rb_seqnum != rb_last_seqnum + 1) {
				fprintf(stderr,"Radioberry internal udp IQ data SEQ ERROR: last %ld, recvd %ld\n", (long) rb_last_seqnum, (long) rb_seqnum);					
			} 
			rb_last_seqnum = rb_seqnum;
		
		
				
			int factor = ((lnrx - 1) * 6) + 8;
			
			int i = 0;
			for (i; i < 171; i++) {
				char *offset = buffer + 4 + (i * 6);
				switch (lnrx)
				{	case 4:
						memcpy(rx_buffer[rx_fill_index] + pointer + 18, offset, 6);
					case 3:
						memcpy(rx_buffer[rx_fill_index] + pointer + 12, offset, 6);
					case 2:
						memcpy(rx_buffer[rx_fill_index] + pointer + 6, offset, 6);
					case 1:
						memcpy(rx_buffer[rx_fill_index] + pointer, offset , 6);
						break;
					default:
						fprintf(stderr, "Should not occur. Number of receivers %d \n", lnrx);
						break;
				}
				pointer = pointer + factor;
				if ( pointer >= 500 || (pointer == 494 && nrx==4) )	{
					rx_fill_index = (rx_fill_index + 1) % RX_MAX; 
					pointer = 0;
					lnrx = nrx;
					sem_post(&rx_full);
				}
			}
		}
		
	}
}

void *spiWriter(void *arg) {
	
	gettimeofday(&t20, 0);
	
	while(1) {
		
		sem_wait(&tx_full);
		


		
		sem_post(&tx_empty); 
		
		
		
		tx_count ++;
		if (tx_count == 48000) {
			tx_count = 0;
			gettimeofday(&t21, 0);
			float elapsd = timedifference_msec(t20, t21);
			fprintf(stderr, "Code tx mode spi executed in %f milliseconds.\n", elapsd);
			gettimeofday(&t20, 0);
		}
	}
}


void spi_control_rx1_phase() {
	unsigned char iqdata[6];
	
	
	rx1_phase = (uint32_t)ceil(((double)freq * 4294967296.0 ) / 76800000);
	
	iqdata[0] = (0x10 | (sampleSpeed & 0x03));
	iqdata[1] = ((dither <<5) & 0x20) |  (att & 0x1F);
	iqdata[2] = ((rx1_phase >> 24) & 0xFF);
	iqdata[3] = ((rx1_phase >> 16) & 0xFF);
	iqdata[4] = ((rx1_phase >> 8) & 0xFF);
	iqdata[5] = (rx1_phase & 0xFF);
	
	sem_wait(&mutex);			
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
	sem_post(&mutex);
}

void spi_control_rx2_phase() {
	unsigned char iqdata[6];
	
	rx2_phase = (uint32_t)ceil(((double)freq2 * 4294967296.0 ) / 76800000);
	
	iqdata[0] = (0x20 | (sampleSpeed & 0x03));
	iqdata[1] = (((rando << 6) & 0x40) | ((dither <<5) & 0x20) |  (att & 0x1F));
	iqdata[2] = ((rx2_phase >> 24) & 0xFF);
	iqdata[3] = ((rx2_phase >> 16) & 0xFF);
	iqdata[4] = ((rx2_phase >> 8) & 0xFF);
	iqdata[5] = (rx2_phase & 0xFF);
			
	sem_wait(&mutex);			
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
	sem_post(&mutex);
}



//inet_aton(radioberry_addr, &src_addr[i].sin_addr);
void getRadioberryInformation() {
	unsigned char data[6];
	memset(data, 0, 6);	
	data[0] = 0x40;
	sem_wait(&mutex);		
	spiXfer(rx1_spi_handler, data, data, 6);
	radioberry_initialized = ((data[0] & 0x80) == 0x80) ? 1: 0;
	if (radioberry_initialized) {
		sprintf(radioberry_addr, "%ld.%ld.%ld.%ld", data[2], data[3], data[4], data[5]);
	}
	
	sem_post(&mutex);
}

void getRunningRadioberryInformation() {
	unsigned char data[6];
	memset(data, 0, 6);	
	data[0] = 0x41;
	sem_wait(&mutex);		
	spiXfer(rx1_spi_handler, data, data, 6);
	radioberry_initialized = ((data[0] & 0x80) == 0x80) ? 1: 0;
	if (radioberry_initialized) {
		fprintf(stderr, "init = %d", radioberry_initialized);
		fprintf(stderr, "# and run = %d", ((data[0] & 0x40) == 0x40) ? 1: 0);
		fprintf(stderr, "# and full fifo = %d", ((data[0] & 0x20) == 0x20) ? 1: 0);
		fprintf(stderr, "# and half full fifo = %d\n", ((data[0] & 0x10) == 0x10) ? 1: 0);
	}
	
	sem_post(&mutex);
}


void stopRadioberryRadio() {
	unsigned char data[6];
	memset(data, 0, 6);	
	data[0] = 0x40;
	sem_wait(&mutex);		
	spiXfer(rx1_spi_handler, data, data, 6);
	fprintf(stderr, "\nStop Radioberry Radio \n");
	sem_post(&mutex);
}

void startRadioberryRadio() {
	unsigned char data[6];
	memset(data, 0, 6);	
	data[0] = 0x41;
	sem_wait(&mutex);		
	spiXfer(rx1_spi_handler, data, data, 6);
	fprintf(stderr, "\nStart Radioberry Radio \n");
	sem_post(&mutex);
}

int createRadioberrySocket() {
	
    struct ifaddrs *addrs,*ifa;
	struct sockaddr_in *sa;
	struct sockaddr_in radioberry;
	memset((char *)&radioberry, 0, sizeof(radioberry));
	radioberry.sin_family = AF_INET;
	//char radioberry_addr[20] = "169.254.45.222";
	inet_aton(radioberry_addr, &radioberry.sin_addr);
	
    struct sockaddr_in *mask;
	static char interface_name[64];
	static char interface_ip_addr[20];
	static char interface_mask[20];

    getifaddrs(&addrs);
    ifa = addrs;
    while (ifa) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
			
            if((ifa->ifa_flags&IFF_UP)==IFF_UP
                && (ifa->ifa_flags&IFF_RUNNING)==IFF_RUNNING
                && (ifa->ifa_flags&IFF_LOOPBACK)!=IFF_LOOPBACK) {

				strcpy(interface_name,ifa->ifa_name);
				fprintf(stderr,"Determine if interface '%s' is suitable for radioberry \n", interface_name);

				sa = (struct sockaddr_in *) ifa->ifa_addr;
				mask = (struct sockaddr_in *) ifa->ifa_netmask;

				strcpy(interface_ip_addr, inet_ntoa(sa->sin_addr));
				fprintf(stderr,"------ip address interface %s\n", interface_ip_addr);
				strcpy(interface_mask, inet_ntoa(mask->sin_addr));
				fprintf(stderr,"------interface mask %s\n", interface_mask);
				
				int sameSubnet = 0;
				if ((sa->sin_addr.s_addr & mask->sin_addr.s_addr)  == (radioberry.sin_addr.s_addr& mask->sin_addr.s_addr)) {fprintf(stderr, "------Suitable\n"); sameSubnet = 1; } else  fprintf(stderr, "------NOT Suitable\n");
				
				
				if (sock_Radioberry == -1 && sameSubnet) {
					fprintf(stderr,"The first found suitable interface will be used for radioberry connection\n");
					 
						/* create a UDP socket */
						if ((sock_Radioberry = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
							perror("cannot create socket\n");
							return -1;
						}
						struct timeval timeout;      
						timeout.tv_sec = 0;
						timeout.tv_usec = TIMEOUT_MS;

						if (setsockopt(sock_Radioberry, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0)
							perror("setsockopt failed\n");
							
						/* bind the socket to any valid IP address and a specific port */
						struct sockaddr_in myaddr;
						memset((char *)&myaddr, 0, sizeof(myaddr));
						myaddr.sin_family = AF_INET;
						myaddr.sin_addr.s_addr = sa->sin_addr.s_addr;
						myaddr.sin_port = htons(1038); 
						
						fprintf(stderr,"RPI IP-address %s  \n", inet_ntoa(myaddr.sin_addr)); 
						fprintf(stderr,"RPI Port %d \n", ntohs(myaddr.sin_port));

						if (bind(sock_Radioberry, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
							perror("bind failed");
							return -1;
						}
						
						usleep(10000);
						
						/*
						//sending udp packet to set radioberry ip, port and mac address in the FPGA.
						unsigned char anydata[30];
						memset(anydata,1,30);
						//struct sockaddr_in rbaddr;	
						rbaddr.sin_family = AF_INET;						
						rbaddr.sin_addr.s_addr = radioberry.sin_addr.s_addr;
						rbaddr.sin_port = htons(1024);
						
						fprintf(stderr,"Radioberry IP-address %s  \n", inet_ntoa(rbaddr.sin_addr)); 
						fprintf(stderr,"Radioberry Port %d \n", ntohs(rbaddr.sin_port));
							
						int bytes = sendto(sock_Radioberry, anydata, sizeof(anydata), 0, (struct sockaddr *)&rbaddr,  sizeof(rbaddr));
						fprintf(stderr, "verzonden %d\n", bytes);
						*/
						break;
					}
            }
        }
        ifa = ifa->ifa_next;
    }
    freeifaddrs(addrs);
	
	return 0; // if no socket than -1 return
}

int aantal =0;
void  receiveudp() {
	int nul = 0;
	fprintf(stderr, "Data streaming .\n");
	while (1) {
		
		
		aantal++;
		unsigned char buffer[2048];
		struct sockaddr_in remaddr;	
		int recvlen = recvfrom(sock_Radioberry, buffer, sizeof(buffer), 0, (struct sockaddr *)&remaddr, &addrlen);
		
		if (recvlen == 1030) {
			
			/*
			int i = 0;
			for (i=0; i < 4; i++) fprintf(stderr, "%2x.", buffer[i]);
			fprintf(stderr,"--");
			for (i=4; i < 18; i++) fprintf(stderr, "%2x.", buffer[i]);
			fprintf(stderr,"--");
			for (i=1023; i < 1030; i++) fprintf(stderr, "%2x.", buffer[i]);
			fprintf(stderr,"\n");
			*/
			rb_seqnum=((buffer[0]&0xFF)<<24)+((buffer[1]&0xFF)<<16)+((buffer[2]&0xFF)<<8)+(buffer[3]&0xFF);
			
			if (rb_seqnum != rb_last_seqnum + 1) {
				fprintf(stderr,"Radioberry internal udp IQ data SEQ ERROR: last %ld, recvd %ld\n", (long) rb_last_seqnum, (long) rb_seqnum);	
				
				int i = 0;
				for (i=0; i < 4; i++) fprintf(stderr, "%2x.", buffer[i]);
					fprintf(stderr,"--");
				for (i=4; i < 18; i++) fprintf(stderr, "%2x.", buffer[i]);
					fprintf(stderr,"--");
				for (i=1023; i < 1030; i++) fprintf(stderr, "%2x.", buffer[i]);
					fprintf(stderr,"\n");
				
			} 
			rb_last_seqnum = rb_seqnum;
			
			if (aantal % 280 == 0) {
				fprintf(stderr, ".");
			//	fprintf(stderr, "Good packet: \n"); 
			//	getRunningRadioberryInformation();
			//	fprintf(stderr, "\n"); 
				//stopRadioberryRadio();
				//sleep(3);
				//startRadioberryRadio();
			}
			
			//unsigned char txiqdata[1030];
			//memset(txiqdata,0,1030);
			//int bytes = sendto(sock_Radioberry, txiqdata, sizeof(txiqdata), 0, (struct sockaddr *)&rbaddr,  sizeof(rbaddr));
			//if (bytes != 1030) fprintf(stderr, "NOT OKverzonden %d\n", bytes);
		
	} else { fprintf(stderr, "Bad packet: \n"); getRunningRadioberryInformation();   sleep(3);}
	//fprintf(stderr, "Aantal verwerkt %d ontvangen fout packet length %d\n", aantal, recvlen);
	}
}


void put_tx_buffer(unsigned char  value) {
    tx_buffer[fill_tx] = value;    
    fill_tx = (fill_tx + 1) % TX_MAX_BUFFER; 
}

unsigned char get_tx_buffer() {
    int tmp = tx_buffer[use_tx];   
    use_tx = (use_tx + 1) % TX_MAX_BUFFER;  	
    return tmp;
}

void printIntroScreen() {
	fprintf(stderr,"\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr, "\tRadioberry V2.0 beta 3. \n");
	fprintf(stderr,	"\n");
	fprintf(stderr,	"\n");
	fprintf(stderr,	"\t\t Radioberry using Ethernet module.\n");
	fprintf(stderr,	"\n");
	fprintf(stderr,	"\n");
	fprintf(stderr, "\tSupporting:\n");
	fprintf(stderr, "\t\t - openhpsdr protocol-1.\n");
	fprintf(stderr, "\t\t - UDP & TCP support for pihpsdr.\n");
	fprintf(stderr, "\t\t - 1 rx receiver. and 0 tx transmitter (full duplex)\n");
	fprintf(stderr,	"\n\n");
	fprintf(stderr, "\t\t\t Have fune Johan PA3GSB\n");
	fprintf(stderr, "\n\n");
	fprintf(stderr, "\n\tReport bugs to <pa3gsb@gmail.com>.\n");
	fprintf(stderr, "====================================================================\n");
	fprintf(stderr, "====================================================================\n");
}

//end of source.