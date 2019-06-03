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
* Tinkerradio. 
*
*	Using the 'HPSDR protocol'; also called protocol-1
*
*  This emulator works with the Radioberry radiocard.
*
*	http://www.pa3gsb.nl
*	  
*	2019 Johan PA3GSB
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

#include "tinkerradio.h"
#include "tinkerboard_io.h"
#include "tinkerboard_spi.h"
#include "tinkerboard_io.c"


struct spi_mode_config_t config_spi = {
	.clk_mode = 3,
	.clk_divider = 4,
	.data_frame_size = DFS_16,
	.slave_select_0 = 24,
	.slave_select_1 = 26,
	.slave_select_2 = 32,
	.transfer_mode = TRANSMIT_RECEIVE,
	.byte_order = MSB_FIRST,
};

float timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
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
	
	if (tinkerboard_init() != 1) {
		perror("radioberry_protocol: tinkerboard IO could not be initialized. \n");
		exit(-1);
	}
		
	tinkerboard_set_gpio_mode(RX1_SAMPLES_PIN, INPUT);  
	tinkerboard_set_gpio_pud(RX1_SAMPLES_PIN, PULLUP);
	tinkerboard_set_gpio_mode(PTT_PIN, OUTPUT); 
	
	tinkerboard_spi_init(SPI2, config_spi);
		
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
	
	if (sock_TCP_Client >= 0)
	{
		close(sock_TCP_Client);
	}

	if (sock_TCP_Server >= 0)
	{
		close(sock_TCP_Server);
	}
	
	tinkerboard_spi_end(SPI2);
    tinkerboard_end();
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

#define assign_change(a,b,c) if ((a) != b) { b = (a); fprintf(stderr, "%20s= %08lx (%10ld)\n", c, (long) b, (long) b ); }

int determine_rx_freq(int base_index, char* buffer) {
	int freq = ( ((buffer[ base_index + 1] & 0xFF) << 24) + ((buffer[ base_index + 2] & 0xFF) << 16) + ((buffer[base_index + 3] & 0xFF) << 8) + (buffer[base_index + 4] & 0xFF) );
	rx1_phase = (uint32_t)ceil(((double)freq * 4294967296.0 ) / 76800000);
	return freq;
}

int determine_tx_freq(int base_index, char* buffer) {
	int freq = ( ((buffer[ base_index + 1] & 0xFF) << 24) + ((buffer[ base_index + 2] & 0xFF) << 16) + ((buffer[base_index + 3] & 0xFF) << 8) + (buffer[base_index + 4] & 0xFF) );
	tx_phase = (uint32_t)ceil(((double)freq * 4294967296.0 ) / 76800000);
	return freq;
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
	if ((buffer[ 11] & 0xFE) == 0x02) assign_change(determine_tx_freq( 11, buffer), txfreq, "TX FREQ");
	if ((buffer[523] & 0xFE) == 0x02) assign_change(determine_tx_freq(523, buffer), txfreq, "TX FREQ");
	if ((buffer[ 11] & 0xFE) == 0x04) assign_change(determine_rx_freq( 11, buffer),   freq, "RX-1 FREQ");
	if ((buffer[523] & 0xFE) == 0x04) assign_change(determine_rx_freq(523, buffer),   freq, "RX-1 FREQ");
	//if ((buffer[ 11] & 0xFE) == 0x06) assign_change(determine_freq( 11, buffer),  freq2, "RX-2 FREQ");
	//if ((buffer[523] & 0xFE) == 0x06) assign_change(determine_freq(523, buffer),  freq2, "RX-2 FREQ");
	
	if ((buffer[523] & 0xFE) == 0x12) assign_change(buffer[524], drive_level, "Drive Level"); 
 
	int frame = 0;
	for (frame; frame < 2; frame++)
	{
		int coarse_pointer = frame * 512 + 8;
		int j = 8;
		for (j; j < 512; j += 8)
		{
			int k = coarse_pointer + j;
			if (MOX) {
				sem_wait(&tx_empty);
				int i = 0;
				for (i; i < 4; i++){
					put_tx_buffer(buffer[k + 4 + i]);	
				}
				sem_post(&tx_full);
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
			
			sem_wait(&rx_full);
			memcpy(hpsdrdata + coarse_pointer + 16, rx_buffer[rx_read_index], 504);
			rx_read_index = (rx_read_index + 1) % RX_MAX; 
			sem_post(&rx_empty);
			
		}
}

void rx1_spiReader(unsigned char iqdata[]) {
		
	iqdata[0] = (sampleSpeed & 0x03);
	iqdata[1] = (((rando << 6) & 0x40) | ((dither <<5) & 0x20) |  (att & 0x1F));
	iqdata[2] = ((rx1_phase >> 24) & 0xFF);
	iqdata[3] = ((rx1_phase >> 16) & 0xFF);
	iqdata[4] = ((rx1_phase >> 8) & 0xFF);
	iqdata[5] = (rx1_phase & 0xFF);
	for (int pointer = 6; pointer < 64 * 6; pointer+=6) memcpy(iqdata + pointer, iqdata, 6);
	sem_wait(&mutex);	
	tinkerboard_spi_transfer(SPI2, iqdata, iqdata, 6 * 64 , config_spi, SS_0);
	sem_post(&mutex);
}

void *spiReader(void *arg) {
	
	int pointer = 0;
	int rx_fill_index = 0;
	int lnrx = nrx;
	
	while(1) {
		
		sem_wait(&rx_empty);
		
		//ptt off
		if (!MOX && saveMox!=MOX) {tinkerboard_set_gpio_state(PTT_PIN, LOW); saveMox = MOX;}
		
		while (tinkerboard_get_gpio_state(RX1_SAMPLES_PIN) == LOW) {} //wait for enough samples
				
		int factor = ((lnrx - 1) * 6) + 8;
		// read 64 IQ samples.
		rx1_spiReader(iqdata);
		int i = 0;
		for (i; i < 64; i++) {
			char *offset = iqdata + i * 6;
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

void *spiWriter(void *arg) {
	
	unsigned char control_data[6];
	gettimeofday(&t20, 0);
	
	while(1) {
		
		sem_wait(&tx_full);
		
		// ptt on
		if (MOX && saveMox!=MOX) {tinkerboard_set_gpio_state(PTT_PIN, HIGH); saveMox = MOX; tx_count = 0;} ;	
		
		if (tx_count % 4800 ==0) {
			//set the tx freq and drive level
			control_data[0] = 0x00;
			control_data[1] = drive_level / 6.4;  // convert drive level from 0-255 to 0-39 )
			if (prev_drive_level != drive_level) {
				fprintf(stderr, "drive level %d - corrected drive level %d \n", drive_level, control_data[1]);
				prev_drive_level = drive_level; 
			}
			control_data[2] = ((tx_phase >> 24) & 0xFF);
			control_data[3] = ((tx_phase >> 16) & 0xFF);
			control_data[4] = ((tx_phase >> 8) & 0xFF);
			control_data[5] = (tx_phase & 0xFF);
						
			if (MOX) {
				sem_wait(&mutex);
				tinkerboard_spi_transfer(SPI2, control_data, control_data, 6, config_spi, SS_2);	
				sem_post(&mutex);				
			}
		}		
		int i = 0;
		for (i; i < 4; i++){			
			tx_iqdata[i] = get_tx_buffer(); //MSB is first in buffer..
		}
		if (MOX) {
			sem_wait(&mutex);
			tinkerboard_spi_transfer(SPI2, tx_iqdata, tx_iqdata, 4, config_spi, SS_1);
			sem_post(&mutex);	
		}
		
		sem_post(&tx_empty); 
		
		if (MOX) {
			int txbufferc = ((tx_iqdata[3] & 0x07) << 8) + (tx_iqdata[4] & 0xFF);
			if (txbufferc > 2044)  usleep(100); //5 samples sleep
		}
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
	fprintf(stderr,	"\t\t Standalone Tinkerboard Version.\n");
	fprintf(stderr,	"\n");
	fprintf(stderr,	"\n");
	fprintf(stderr, "\tSupporting:\n");
	fprintf(stderr, "\t\t - openhpsdr protocol-1.\n");
	fprintf(stderr, "\t\t - UDP & TCP support for pihpsdr.\n");
	fprintf(stderr, "\t\t - Full duplex. Rx and Tx channel. \n");
	fprintf(stderr, "\t\t - initial version for tinkerboard no support yet for vna - cw - i2c - filter \n");
	fprintf(stderr,	"\n\n");
	fprintf(stderr, "\t\t\t Have fune Johan PA3GSB\n");
	fprintf(stderr, "\n\n");
	fprintf(stderr, "\n\tReport bugs to <pa3gsb@gmail.com>.\n");
	fprintf(stderr, "====================================================================\n");
	fprintf(stderr, "====================================================================\n");
}

//end of source.