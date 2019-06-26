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
*	Radioberry radiocard is a so called 'HAT' for a raspberry pi; working together resulting in a HAM radio.
*
*	Radioberry v2.0 SDR firmware code. (small hw mod required and only FPGA CL025)
*
*	The radioberry firmware is formed by:
*			- verilog radioberry.rbf FPGA image,
*			- this radioberry c code part, implementing the openHPSDR protocol-1, running at a Raspberry PI.
*
*	http://www.pa3gsb.nl
*	  
*	Johan PA3GSB
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#include <pigpio.h>
#include "radioberry.h"


int main(int argc, char **argv)
{
	printIntroScreen();
	
	if (initRadioberry() < 0){
		fprintf(stderr,"Radioberry; could not be initialized. \n");
		exit(-1);
	}
	runRadioberry();
	closeRadioberry();
}

int initRadioberry() {
	sem_init(&mutex, 0, 1);	
	sem_init(&tx_empty, 0, TX_MAX); 
    sem_init(&tx_full, 0, 0);    	
	
	if (gpioInitialise() < 0) {
		fprintf(stderr,"Radioberry;  gpio could not be initialized. \n");
		exit(-1);
	}
	
	gpioSetMode(25, PI_INPUT); 	//rx samples
	
	// defining the IO ports as input; required for reading bits in one cycle.
	// rx data byte:
	gpioSetMode(23, PI_INPUT); 
	gpioSetMode(20, PI_INPUT);
	gpioSetMode(19, PI_INPUT);
	gpioSetMode(18, PI_INPUT);
	gpioSetMode(16, PI_INPUT);
	gpioSetMode(13, PI_INPUT);
	gpioSetMode(12, PI_INPUT);
	gpioSetMode(5, PI_INPUT); 

	//define control IO ports
	gpioSetMode(6, PI_OUTPUT);  	// pi-clk rx1 slice
	gpioSetMode(17, PI_OUTPUT); 	// pi-clk rx2 slice
	gpioSetMode(21, PI_OUTPUT);  	// ptt
	
	//Initialise
	gpioWrite(21, 0); 				// set ptt off
	gpioWrite(6, 0); 				// init pi-clk1 
	gpioWrite(17, 0); 				// init pi-clk2 
	
	// open and initialize the 
	i2c_handler = i2cOpen(i2c_bus, MAX11613_ADDRESS, 0);
	if (i2c_handler >= 0)  initVSWR();
	
	rx1_spi_handler = spiOpen(0, 15625000, 49155);  //channel 0
	if (rx1_spi_handler < 0) {
		fprintf(stderr,"radioberry_protocol: spi bus rx1 could not be initialized. \n");
		exit(-1);
	}
	
	rx2_spi_handler = spiOpen(1, 15625000, 49155); 	//channel 1
	if (rx2_spi_handler < 0) {
		fprintf(stderr,"radioberry_protocol: spi bus rx2 could not be initialized. \n");
		exit(-1);
	}
	
	printf("Radioberry, Initialisation succesfully executed.\n");
		
	pthread_t pid1, pid2; 
	pthread_create(&pid1, NULL, packetreader, NULL); 
	pthread_create(&pid2, NULL, spiWriter, NULL);

	/* create a UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
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
		return 0;
	}
	
	if ((sock_TCP_Server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket tcp");
		return -1;
	}
	int yes = 1;
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_REUSEADDR, (void *)&yes, sizeof(yes));
	int sndbufsize = 0xffff;
	int rcvbufsize = 0xffff;
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_SNDBUF, (const char *)&sndbufsize, sizeof(int));
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVBUF, (const char *)&rcvbufsize, sizeof(int));

	if (bind(sock_TCP_Server, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
	{
		perror("bind tcp");
		return -1;
	}
	listen(sock_TCP_Server, 1024);
}

void closeRadioberry() {
	if (rx1_spi_handler !=0) spiClose(rx1_spi_handler);
	if (rx2_spi_handler !=0) spiClose(rx2_spi_handler);
	if (sock_TCP_Client >= 0) close(sock_TCP_Client);
	if (sock_TCP_Server >= 0) close(sock_TCP_Server);
		
	gpioTerminate();
}

void runRadioberry() {
	fprintf(stderr, "Radioberry, Starting packet tx part. \n");
	while(1) {
		if (running) {
			sendPacket();
		} else {usleep(20000);}
	}
}
void *packetreader(void *arg) {
	int size, bytes_read, bytes_left;
	unsigned char buffer[2048];
	uint32_t *code0 = (uint32_t *) buffer; 
	fprintf(stderr, "Radioberry, Starting packet rx part. \n");
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
			if (recvlen > 0) handlePacket(buffer);
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
			fillDiscoveryReplyMessage();
			if (sendto(fd, broadcastReply, sizeof(broadcastReply), 0, (struct sockaddr *)&remaddr, addrlen) < 0) fprintf(stderr, "broadcast reply error");
			
			break;
		case 0x0004feef:
			fprintf(stderr, "SDR Program sends Stop command \n");
			running = 0;
			last_sequence_number = 0;
			if (sock_TCP_Client > -1)
			{
				close(sock_TCP_Client);
				sock_TCP_Client = -1;
				fprintf(stderr, "SDR Program sends TCP Stop command \n");
			} else fprintf(stderr, "SDR Program sends UDP Stop command \n");	
			break;
		case 0x0104feef:
		case 0x0304feef:
			fprintf(stderr, "Start Port %d \n", ntohs(remaddr.sin_port));
			running = 1;
			fprintf(stderr, "SDR Program sends UDP Start command \n");
			break;
		case 0x1104feef: 
			fprintf(stderr, "Connect the TCP client to the server\n");
			if (sock_TCP_Client < 0)
			{
				if((sock_TCP_Client = accept(sock_TCP_Server, NULL, NULL)) < 0)
				{
					fprintf(stderr, "*** ERROR TCP accept ***\n");
					perror("accept");
					return;
				}
				fprintf(stderr, "sock_TCP_Client: %d connected to sock_TCP_Server: %d\n", sock_TCP_Client, sock_TCP_Server);
				running = 1;
				fprintf(stderr, "SDR Program sends TCP Start command \n");
			}	
			break;
		case 0x0201feef:
			processPacket(buffer);
			break;		
	}
}

void processPacket(char* buffer)
{
	if (!vna) {
		//bug in hermes vna seq nummer is always 0. In VNA mode seq number is not that important!
		seqnum=((buffer[4]&0xFF)<<24)+((buffer[5]&0xFF)<<16)+((buffer[6]&0xFF)<<8)+(buffer[7]&0xFF);
		if (seqnum != last_seqnum + 1) {
		  fprintf(stderr,"Radioberry firmware SEQ ERROR: last %ld, recvd %ld\n", (long) last_seqnum, (long) seqnum);
		}
		last_seqnum = seqnum;
	}

	 MOX = ((buffer[11] & 0x01)==0x01) ? 1:0;
	 if (MOX) gpioWrite(21, 1); else gpioWrite(21, 0);	// ptt 

	if ((buffer[11] & 0xFE)  == 0x22) {
		pwmin = ((buffer[11+1] & 0xFF) << 2) + (buffer[11 + 2] & 0x03);
		pwmax = ((buffer[11+3] & 0xFF) << 2) + (buffer[11 + 4] & 0x03);
	}
	
	if ((buffer[11] & 0xFE)  == 0x14) {
		att = (buffer[11 + 4] & 0x1F);
		att11 = att;
		pureSignal = ((buffer[11+2] & 0x40) == 0x40)? 0x01 : 0x00;
	}
	
	if ((buffer[523] & 0xFE)  == 0x14) {
		att = (buffer[523 + 4] & 0x1F);
		att523 = att;
	}
	
	if ((buffer[11] & 0xFE)  == 0x00) {
		nrx = (((buffer[11 + 4] & 0x38) >> 3) + 1);
		
		sampleSpeed = (buffer[11 + 1] & 0x03);
		if (sampleSpeed != holdsampleSpeed) {
			printf("sample speed   %d \n", sampleSpeed);
			holdsampleSpeed = sampleSpeed;
			change = change | 0x03; // in Protocol-1 valid for rx1 and rx2
		}
		dither = 0;
		if ((buffer[11 + 3] & 0x08) == 0x08) dither = 1; 		
		rando = 0;
		if ((buffer[11 + 3] & 0x10) == 0x10) rando = 1;
		vna_att_20dB = 0;
		if ((buffer[11 + 3] & 0x03) == 0x02) vna_att_20dB = 1;
		
	}
		
	if ((buffer[523] & 0xFE)  == 0x00) {
		dither = 0;
		if ((buffer[523 + 3] & 0x08) == 0x08) dither = 1; 	
		rando = 0;
		if ((buffer[523 + 3] & 0x10) == 0x10) rando = 1;
	}
	if (prevatt11 != att11) 
	{
		att = att11;
		prevatt11 = att11;
	}
	if (prevatt523 != att523) 
	{
		att = att523;
		prevatt523 = att523;
	}
			
	if ((buffer[11] & 0xFE)  == 0x00) {
		nrx = (((buffer[11 + 4] & 0x38) >> 3) + 1);
	}
	if ((buffer[523] & 0xFE)  == 0x00) {
		nrx = (((buffer[523 + 4] & 0x38) >> 3) + 1);
	}
	if (hold_nrx != nrx) {
		hold_nrx=nrx;
		printf("aantal rx %d \n", nrx);
	}
	
	// select Command
	if ((buffer[11] & 0xFE) == 0x02)
	{
		txfreq = ((buffer[11 + 1] & 0xFF) << 24) + ((buffer[11+ 2] & 0xFF) << 16)
				+ ((buffer[11 + 3] & 0xFF) << 8) + (buffer[11 + 4] & 0xFF);
				
		tx_phase = (uint32_t)ceil(((double)txfreq * 4294967296.0 ) / 76800000);
		
		if (vna) {
			freq = txfreq;
			rx1_phase = (uint32_t)ceil(((double)freq * 4294967296.0 ) / 76800000);
		}
		
	}
	if ((buffer[523] & 0xFE) == 0x02)
	{
		txfreq = ((buffer[523 + 1] & 0xFF) << 24) + ((buffer[523+ 2] & 0xFF) << 16)
				+ ((buffer[523 + 3] & 0xFF) << 8) + (buffer[523 + 4] & 0xFF);
		tx_phase = (uint32_t)ceil(((double)txfreq * 4294967296.0 ) / 76800000);
		
		if (vna) {
			freq = txfreq;
			rx1_phase = (uint32_t)ceil(((double)freq * 4294967296.0 ) / 76800000);
		}
	}
	
	if ((buffer[11] & 0xFE) == 0x04)
	{
		freq = ((buffer[11 + 1] & 0xFF) << 24) + ((buffer[11+ 2] & 0xFF) << 16)
				+ ((buffer[11 + 3] & 0xFF) << 8) + (buffer[11 + 4] & 0xFF);
		rx1_phase = (uint32_t)ceil(((double)freq * 4294967296.0 ) / 76800000);
	}
	if ((buffer[523] & 0xFE) == 0x04)
	{
		freq = ((buffer[523 + 1] & 0xFF) << 24) + ((buffer[523+ 2] & 0xFF) << 16)
				+ ((buffer[523 + 3] & 0xFF) << 8) + (buffer[523 + 4] & 0xFF);
		rx1_phase = (uint32_t)ceil(((double)freq * 4294967296.0 ) / 76800000);
	}
		
	if ((buffer[11] & 0xFE) == 0x06)
	{
		freq2 = ((buffer[11 + 1] & 0xFF) << 24) + ((buffer[11+ 2] & 0xFF) << 16)
				+ ((buffer[11 + 3] & 0xFF) << 8) + (buffer[11 + 4] & 0xFF);
		rx2_phase = (uint32_t)ceil(((double)freq2 * 4294967296.0 ) / 76800000);
	}
	if ((buffer[523] & 0xFE) == 0x06)
	{
		freq2 = ((buffer[523 + 1] & 0xFF) << 24) + ((buffer[523+ 2] & 0xFF) << 16)
				+ ((buffer[523 + 3] & 0xFF) << 8) + (buffer[523 + 4] & 0xFF);
		rx2_phase = (uint32_t)ceil(((double)freq2 * 4294967296.0 ) / 76800000);
	}

	// select Command
	if ((buffer[11] & 0xFE) == 0x12) {
		vna = (buffer[13] & 0x80) == 0x80 ? 1: 0;
	}
	//if ((buffer[11] & 0xFE) == 0x12) drive_level = buffer[14]; 
	if ((buffer[523] & 0xFE) == 0x12) drive_level = buffer[524];  
	
	
	if ((prev_pwim != pwmin) || (prev_pwmax != pwmax) || (holdPureSignal != pureSignal)) {
		printf("pwmmin =  %d  en pwmmax = %d \n", pwmin, pwmax);
		fprintf(stderr, "Puresignal status %d\n", pureSignal);
		change = change | 0x08;
		prev_pwim = pwmin;
		prev_pwmax = pwmax;
		holdPureSignal = pureSignal;
	}
	if (!vna && (holdatt != att) || (holddither != dither)) {
		holdatt = att;
		holddither = dither;
		printf("att =  %d ", att);printf("dither =  %d ", dither);printf("rando =  %d ", rando);
		printf("code =  %d \n", (((rando << 6) & 0x40) | ((dither <<5) & 0x20) |  (att & 0x1F)));
		printf("att11 = %d and att523 = %d\n", att11, att523);
		change = change | 0x01;
	}
	if (holdfreq != freq) {
		holdfreq = freq;
		printf("frequency rx 1 %d en aantal rx %d \n", freq, nrx);
		printf("frequency rx-phase 1 %d en aantal rx %d \n", rx1_phase, nrx);
		change = change | 0x01;
	}
	if (holdfreq2 != freq2) {
		holdfreq2 = freq2;
		printf("frequency rx 2 %d en aantal rx %d \n", freq2, nrx);
		printf("frequency rx-phase 2 %d en aantal rx %d \n", rx2_phase, nrx);
		change = change | 0x02;
	}
	if (!vna && (holdtxfreq != txfreq) || (drive_level != prev_drive_level)) {
		holdtxfreq = txfreq;
		prev_drive_level = drive_level;
		fprintf(stderr, "TX frequency %d\n", txfreq);
		fprintf(stderr, "Drive level %d\n", drive_level);
		change = change | 0x04;
	}
	
	if ((holdvna != vna) || (holdvna_att_20dB != vna_att_20dB)) {
		fprintf(stderr, "VNA Mode %d\n", vna);
		if (vna) {
			if (vna_att_20dB) att = 57; else att = 45;
			dither = 1; 
			rando = 0;
			//drive_level=137;
			change = change | 0x04;
		}
		change = change | 0x01;
		holdvna = vna;
		holdvna_att_20dB = vna_att_20dB;
	}
	
	// The change must be communicated to radioberry firmware
	// The change var filling determines which spi control must be called.
	if (change != 0x00) {
		if ((change & 0x01) == 0x01) spi_control_rx1_phase();
		if ((change & 0x02) == 0x02) spi_control_rx2_phase();
		if ((change & 0x04) == 0x04) spi_control_tx();
		if ((change & 0x08) == 0x08) spi_send_control();
		change = 0x00; 
	}
		
	int frame = 0;
	for (frame; frame < 2; frame++)
	{
		int coarse_pointer = frame * 512 + 8;
		int j = 8;
		for (j; j < 512; j += 8)
		{
			int k = coarse_pointer + j;
			if (MOX && !vna) {
				sem_wait(&tx_empty);
				int i = 0;
				for (i; i < 8; i++){
					put_tx_buffer(buffer[k + i]);	
				}
				sem_post(&tx_full);
			}
		}
		
	}
}

void sendPacket() {
	fillPacketToSend();
	if (sock_TCP_Client >= 0) {
		if (sendto(sock_TCP_Client, hpsdrdata, sizeof(hpsdrdata), 0, NULL, 0) != 1032) fprintf(stderr, "TCP send error");
	} else {
		if (sendto(fd, hpsdrdata, sizeof(hpsdrdata), 0, (struct sockaddr *)&remaddr, addrlen) != 1032) fprintf(stderr, "UDP send error");
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

		int factor = (nrx - 1) * 6;
		int index=0;
		int frame = 0;
		for (frame; frame < 2; frame++) {
			int coarse_pointer = frame * 512; // 512 bytes total in each frame
				
			while (gpioRead(25) == 0) {usleep(3000);}//wait for enough samples
			
			int i = 0;
			for (i=0; i< (504 / (8 + factor)); i++) {
				index = 16 + coarse_pointer + (i * (8 + factor));
				rx_reader(iqdata);
				int j =0;
				for (j; j< 6; j++){
					hpsdrdata[index + j] = iqdata[j];							//rx1
					if (nrx == 4) hpsdrdata[index + j + 12] =  iqdata[j]; 		//rx3 (PS-RX-FEEDBACK; actual tx signal)
				}
				if (nrx > 1) {
					rx2_reader(iqdata);
					int j =0;
					for (j; j< 6; j++){
						hpsdrdata[index + j + 6] =  iqdata[j]; 					//rx2
						if (nrx == 4) hpsdrdata[index + j + 18] =  iqdata[j]; 	//rx4 (PS-TX-FEEDBACK; DAC values used for tx)
					}	 
				}
			}
		
			if (MOX){
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
			}
		}
}

void fillDiscoveryReplyMessage() {
	int i = 0;
	for (i; i < 60; i++) {
		broadcastReply[i] = 0x00;
	}
	i = 0;
	broadcastReply[i++] = 0xEF;
	broadcastReply[i++] = 0xFE;
	broadcastReply[i++] = 0x02;

	broadcastReply[i++] =  0x00; // MAC
	broadcastReply[i++] =  0x01;
	broadcastReply[i++] =  0x02;
	broadcastReply[i++] =  0x03;
	broadcastReply[i++] =  0x04;
	broadcastReply[i++] =  0x05;
	broadcastReply[i++] =  40;
	broadcastReply[i++] =  6; // hermeslite
									
}

int level = 0;
void rx_reader(unsigned char iqdata[]){
	int i = 0;
	for (i; i < 6 ; i++) {
		level = !level;
		gpioWrite(6, level);
		
		uint32_t value = gpioRead_Bits_0_31();
		value = gpioRead_Bits_0_31();
		
		iqdata[i]  =  (((value >> 23) & 1) << 7);
		iqdata[i] |=  (((value >> 20) & 1) << 6);
		iqdata[i] |=  (((value >> 19) & 1) << 5);
		iqdata[i] |=  (((value >> 18) & 1) << 4);
		iqdata[i] |=  (((value >> 16) & 1) << 3);
		iqdata[i] |=  (((value >> 13) & 1) << 2);
		iqdata[i] |=  (((value >> 12) & 1) << 1);
		iqdata[i] |=  (((value >> 5) & 1));
	}
}

int level2 = 0;
void rx2_reader(unsigned char iqdata[]){
	int i = 0;
	for (i; i < 6 ; i++) {
		level2 = !level2;
		gpioWrite(17, level2);
		
		uint32_t value = gpioRead_Bits_0_31();
		value = gpioRead_Bits_0_31();
		
		iqdata[i]  =  (((value >> 23) & 1) << 7);
		iqdata[i] |=  (((value >> 20) & 1) << 6);
		iqdata[i] |=  (((value >> 19) & 1) << 5);
		iqdata[i] |=  (((value >> 18) & 1) << 4);
		iqdata[i] |=  (((value >> 16) & 1) << 3);
		iqdata[i] |=  (((value >> 13) & 1) << 2);
		iqdata[i] |=  (((value >> 12) & 1) << 1);
		iqdata[i] |=  (((value >> 5) & 1));
	}
}

void spi_control_rx1_phase() {
	unsigned char iqdata[6];
	
	iqdata[0] = (0x10 | (sampleSpeed & 0x03));
	iqdata[1] = (((vna << 7) & 0x80) | ((rando << 6) & 0x40) | ((dither <<5) & 0x20) |  (att & 0x1F));
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

void spi_control_tx() {
	unsigned char iqdata[6];
	
	iqdata[0] = 0x30;
	iqdata[1] = drive_level / 6.4;  // convert drive level from 0-255 to 0-39 )
	iqdata[2] = ((tx_phase >> 24) & 0xFF);
	iqdata[3] = ((tx_phase >> 16) & 0xFF);
	iqdata[4] = ((tx_phase >> 8) & 0xFF);
	iqdata[5] = (tx_phase & 0xFF);
		
	sem_wait(&mutex);		
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
	sem_post(&mutex);
}

void spi_send_control() {
	unsigned char iqdata[6];
	
	iqdata[0] = 0x40;
	iqdata[1] = pureSignal;
	iqdata[2] = ((pwmax >> 8) & 0x03);
	iqdata[3] = (pwmax & 0xFF);
	iqdata[4] = ((pwmin >> 8) & 0x03);
	iqdata[5] = (pwmin & 0xFF);
		
	sem_wait(&mutex);		
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
	sem_post(&mutex);
}

int low=0; 
int high=0;
long total= 0;
long totalER=0;
int lowER=0; 
int highER=0;

void *spiWriter(void *arg) {
	
	gettimeofday(&t20, 0);
	
	while(1) {
		
		sem_wait(&tx_full);
  
		int i = 0;
		for (i; i < 8; i++){			
			tx_iqdata[i] = get_tx_buffer(); //EER first 4 bytes followed by 4 bytes TX IQ.
		}
		if (MOX) {
			sem_wait(&mutex);
			spiXfer(rx2_spi_handler, tx_iqdata, tx_iqdata, 8);
			sem_post(&mutex);
		}
		sem_post(&tx_empty); 
		
		 int txbufferc = ((tx_iqdata[6] & 0x07) << 8) + (tx_iqdata[7] & 0xFF);
		 total = total + txbufferc;
		 int txERbufferc = ((tx_iqdata[4] & 0x07) << 8) + (tx_iqdata[5] & 0xFF);
		 totalER = totalER + txERbufferc;
		 if (txbufferc==0)low++;
		 if (txbufferc > 2044)  {
			 usleep(100); //5samples sleep
			high++;
		} 
		if (txERbufferc==0) lowER++;
		if (txERbufferc> 2044) {	highER++;   }//5samples sleep
		
		
		tx_count ++;
		if (tx_count == 48000) {
			tx_count = 0;
			gettimeofday(&t21, 0);
			float elapsd = timedifference_msec(t20, t21);
			fprintf(stderr, "Code tx mode spi executed in %f milliseconds.\n", elapsd);
			fprintf(stderr, "Average firmware buffer size %d low %d and high %d \n", ((int) (total / 48000)), low, high);
			fprintf(stderr, "Average firmware EER buffer size %d low %d and high %d\n", ((int) (totalER / 48000)), lowER, highER);
			total = 0;low=0; high = 0; totalER=0; lowER=0; highER = 0;
			gettimeofday(&t20, 0);
		}
	}
}

void put_tx_buffer(unsigned char  value) {
    tx_buffer[fill_tx] = value;    
    fill_tx = (fill_tx + 1) % TX_MAX_BUFFER; 
}

unsigned char get_tx_buffer() {
    unsigned char tmp = tx_buffer[use_tx];   
    use_tx = (use_tx + 1) % TX_MAX_BUFFER;  	
    return tmp;
}

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

void printIntroScreen() {
	fprintf(stderr,"\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr, "\tRadioberry V2.0 beta 2 and beta 3 (incl. small mod).\n");
	fprintf(stderr,	"\n");
	fprintf(stderr, "\tSupporting:\n");
	fprintf(stderr, "\t\t - openhpsdr protocol-1.\n");
	fprintf(stderr, "\t\t - full duplex mode.\n");
	fprintf(stderr, "\t\t - VNA mode (supports Hermes VNA).\n");
	fprintf(stderr, "\t\t - pure signal.\n");
	fprintf(stderr, "\t\t - EER. (Envelope Elimination and Restoration) (only CL025) \n");
	fprintf(stderr, "\t\t - 2rx slices max 192K sampling rate\n");
	fprintf(stderr, "\t\t - TCP mode in piHPSDR\n");
	fprintf(stderr, "\t\t - VSWR monitor (https://github.com/pa3gsb/vswr)\n");
	fprintf(stderr,	"\n\n");
	fprintf(stderr, "\t\t\t Have fune Johan PA3GSB\n");
	fprintf(stderr, "\n\n");
	fprintf(stderr, "\n\tReport bugs to <pa3gsb@gmail.com>.\n");
	fprintf(stderr, "====================================================================\n");
	fprintf(stderr, "====================================================================\n");
}

//end of source.