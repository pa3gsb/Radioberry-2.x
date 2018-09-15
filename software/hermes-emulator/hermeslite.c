/* 
* Copyright (C)
* 2017, 2018 - Johan Maas, PA3GSB
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
* Emulator. By using this emulator you have the possibility to connect to SDR programs like:
*	- Quisk
*	- PowerSDR
*	- Spark
*
*	Using the 'old HPSDR protocol'
*
*  This emulator works with the Radioberry radiocard.
*
*	http://www.pa3gsb.nl
*	  
*	2018 Johan PA3GSB
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>

#include <pigpio.h>

void runHermesLite(void);
void sendPacket(void);
void handlePacket(char* buffer);
void readPackets(void);
void fillDiscoveryReplyMessage(void);
int isValidFrame(char* data);
void fillPacketToSend(void);
void init(void);
void *spiReader(void *arg);
void *packetreader(void *arg);
void *spiWriter(void *arg);
void put_tx_buffer(unsigned char  value);
unsigned char get_tx_buffer(void);

#define TX_MAX 9600 
unsigned char tx_buffer[TX_MAX * 4];
int fill_tx = 0; 
int use_tx  = 0;
unsigned char drive_level;
unsigned char prev_drive_level;
int MOX = 0;
sem_t tx_empty;
sem_t tx_full;
sem_t mutex;

void rx1_spiReader(unsigned char iqdata[]);
void rx2_spiReader(unsigned char iqdata[]);

static int rx1_spi_handler;
static int rx2_spi_handler;

unsigned char iqdata[6];
unsigned char tx_iqdata[6];

#define SERVICE_PORT	1024

int hold_nrx=0;
int nrx = 2; // n Receivers
int holdfreq = 0;
int holdfreq2 = 0;
int holdtxfreq = 0;
int freq = 4706000;
int freq2 = 1008000;
int txfreq = 3630000;

int att = 0;
int holdatt =128;
int holddither=128;
int dither = 0;
int rando = 0;
int sampleSpeed = 0;

unsigned char SYNC = 0x7F;
int last_sequence_number = 0;

unsigned char hpsdrdata[1032];
unsigned char broadcastReply[60];
#define TIMEOUT_MS      100     

int running = 0;
int fd;									/* our socket */

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

int main(int argc, char **argv)
{
	fprintf(stderr,"\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr, "\t\t\t Radioberry V2.0 beta 2.\n");
	fprintf(stderr,	"\n");
	fprintf(stderr, "\t\t\t Emulator version 9-15-2018 \n");
	fprintf(stderr,	"\n\n");
	fprintf(stderr, "\t\t\t Have fune Johan PA3GSB\n");
	fprintf(stderr, "\n\n");
	fprintf(stderr, "\n\tReport bugs to <pa3gsb@gmail.com>.\n");
	fprintf(stderr, "====================================================================\n");
	fprintf(stderr, "====================================================================\n");
	
	sem_init(&mutex, 0, 1);	
	sem_init(&tx_empty, 0, TX_MAX); 
    sem_init(&tx_full, 0, 0);    	
	
	if (gpioInitialise() < 0) {
		fprintf(stderr,"hpsdr_protocol (original) : gpio could not be initialized. \n");
		exit(-1);
	}
	
	gpioSetMode(13, PI_INPUT); 	//rx1 samples
	gpioSetMode(16, PI_INPUT);	//rx2 samples 
	gpioSetMode(21, PI_OUTPUT); 
	
		
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

	printf("init done \n");
		
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
	runHermesLite();
	
	if (rx1_spi_handler !=0)
		spiClose(rx1_spi_handler);
	if (rx2_spi_handler !=0)
		spiClose(rx2_spi_handler);
		
	gpioTerminate();
}

void runHermesLite() {
	printf("runHermesLite \n");

	for (;;) {
		if (running) {
			sendPacket();
		} else {usleep(20000);}
	}
}
void *packetreader(void *arg) {
	while(1) {
		readPackets();
	}
}

void readPackets() {
	unsigned char buffer[2048];
	
	recvlen = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&remaddr, &addrlen);
	if (recvlen > 0) 
		handlePacket(buffer);
}

int att11 = 0;
int prevatt11 = 0;
int att523 = 0;
int prevatt523 = 0;

void handlePacket(char* buffer){

	if (buffer[2] == 2) {
		printf("Discovery packet received \n");
		printf("IP-address %d.%d.%d.%d  \n", 
							remaddr.sin_addr.s_addr&0xFF,
                            (remaddr.sin_addr.s_addr>>8)&0xFF,
                            (remaddr.sin_addr.s_addr>>16)&0xFF,
                            (remaddr.sin_addr.s_addr>>24)&0xFF);
		printf("Discovery Port %d \n", ntohs(remaddr.sin_port));
		
		fillDiscoveryReplyMessage();
		
		if (sendto(fd, broadcastReply, sizeof(broadcastReply), 0, (struct sockaddr *)&remaddr, addrlen) < 0)
			printf("error sendto");
		
	} else if (buffer[2] == 4) {
			if (buffer[3] == 1 || buffer[3] == 3) {
				printf("Start Port %d \n", ntohs(remaddr.sin_port));
				running = 1;
				printf("SDR Program sends Start command \n");
				return;
			} else {
				running = 0;
				last_sequence_number = 0;
				printf("SDR Program sends Stop command \n");
				return;
			}
		}
	if (isValidFrame(buffer)) {
	
		 MOX = ((buffer[11] & 0x01)==0x01) ? 1:0;
	
		if ((buffer[11] & 0xFE)  == 0x14) {
			att = (buffer[11 + 4] & 0x1F);
			att11 = att;
		}
		
		if ((buffer[523] & 0xFE)  == 0x14) {
			att = (buffer[523 + 4] & 0x1F);
			att523 = att;
		}
	
		if ((buffer[11] & 0xFE)  == 0x00) {
			nrx = (((buffer[11 + 4] & 0x38) >> 3) + 1);
			
			sampleSpeed = (buffer[11 + 1] & 0x03);
			
			dither = 0;
			if ((buffer[11 + 3] & 0x08) == 0x08)
				dither = 1; 
						
			rando = 0;
			if ((buffer[11 + 3] & 0x10) == 0x10)
				rando = 1;
		}
		
		if ((buffer[523] & 0xFE)  == 0x00) {
			
			dither = 0;
			if ((buffer[523 + 3] & 0x08) == 0x08)
				dither = 1; 
					
			rando = 0;
			if ((buffer[523 + 3] & 0x10) == 0x10)
				rando = 1;
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
        }
        if ((buffer[523] & 0xFE) == 0x02)
        {
            txfreq = ((buffer[523 + 1] & 0xFF) << 24) + ((buffer[523+ 2] & 0xFF) << 16)
                    + ((buffer[523 + 3] & 0xFF) << 8) + (buffer[523 + 4] & 0xFF);
        }
		
		if ((buffer[11] & 0xFE) == 0x04)
        {
            freq = ((buffer[11 + 1] & 0xFF) << 24) + ((buffer[11+ 2] & 0xFF) << 16)
                    + ((buffer[11 + 3] & 0xFF) << 8) + (buffer[11 + 4] & 0xFF);
        }
        if ((buffer[523] & 0xFE) == 0x04)
        {
            freq = ((buffer[523 + 1] & 0xFF) << 24) + ((buffer[523+ 2] & 0xFF) << 16)
                    + ((buffer[523 + 3] & 0xFF) << 8) + (buffer[523 + 4] & 0xFF);
        }
		
		if ((buffer[11] & 0xFE) == 0x06)
        {
            freq2 = ((buffer[11 + 1] & 0xFF) << 24) + ((buffer[11+ 2] & 0xFF) << 16)
                    + ((buffer[11 + 3] & 0xFF) << 8) + (buffer[11 + 4] & 0xFF);
        }
        if ((buffer[523] & 0xFE) == 0x06)
        {
            freq2 = ((buffer[523 + 1] & 0xFF) << 24) + ((buffer[523+ 2] & 0xFF) << 16)
                    + ((buffer[523 + 3] & 0xFF) << 8) + (buffer[523 + 4] & 0xFF);
        }

        // select Command
        if ((buffer[523] & 0xFE) == 0x12)
        {
            drive_level = buffer[524];  
        }
		
		if ((holdatt != att) || (holddither != dither)) {
			holdatt = att;
			holddither = dither;
			printf("att =  %d ", att);printf("dither =  %d ", dither);printf("rando =  %d ", rando);
			printf("code =  %d \n", (((rando << 6) & 0x40) | ((dither <<5) & 0x20) |  (att & 0x1F)));
			printf("att11 = %d and att523 = %d\n", att11, att523);
		}
		if (holdfreq != freq) {
			holdfreq = freq;
			printf("frequency %d en aantal rx %d \n", freq, nrx);
		}
		if (holdfreq2 != freq2) {
			holdfreq2 = freq2;
			printf("frequency %d en aantal rx %d \n", freq2, nrx);
		}
		if (holdtxfreq != txfreq) {
			holdtxfreq = txfreq;
			printf("TX frequency %d\n", txfreq);
		}
		
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
}

void sendPacket() {
	fillPacketToSend();
	
	if (sendto(fd, hpsdrdata, sizeof(hpsdrdata), 0, (struct sockaddr *)&remaddr, addrlen) < 0)
			printf("error sendto");
}


int isValidFrame(char* data) {
	return (data[8] == SYNC && data[9] == SYNC && data[10] == SYNC && data[520] == SYNC && data[521] == SYNC && data[522] == SYNC);
}

void fillPacketToSend() {
		
		hpsdrdata[0] = 0xEF;
		hpsdrdata[1] = 0xFE;
		hpsdrdata[2] = 0x01;
		hpsdrdata[3] = 0x06;
		hpsdrdata[4] = ((last_sequence_number >> 24) & 0xFF);
		hpsdrdata[5] = ((last_sequence_number >> 16) & 0xFF);
		hpsdrdata[6] = ((last_sequence_number >> 8) & 0xFF);
		hpsdrdata[7] = (last_sequence_number & 0xFF);
		last_sequence_number++;

		int factor = (nrx - 1) * 6;
		int index=0;
		int frame = 0;
		for (frame; frame < 2; frame++) {
			int coarse_pointer = frame * 512; // 512 bytes total in each frame
			hpsdrdata[8 + coarse_pointer] = SYNC;
			hpsdrdata[9 + coarse_pointer] = SYNC;
			hpsdrdata[10 + coarse_pointer] = SYNC;
			hpsdrdata[11 + coarse_pointer] = 0x00; // c0
			hpsdrdata[12 + coarse_pointer] = 0x00; // c1
			hpsdrdata[13 + coarse_pointer] = 0x00; // c2
			hpsdrdata[14 + coarse_pointer] = 0x00; // c3
			hpsdrdata[15 + coarse_pointer] = 0x28; // c4 //v4.0 firmware version

			if (!MOX) {
				
				sem_wait(&mutex); 
				
				gpioWrite(21, 0); 	// ptt off
				
				while (gpioRead(13) == 0) {}//wait for enough samples
				
				int i = 0;
				for (i=0; i< (504 / (8 + factor)); i++) {
					index = 16 + coarse_pointer + (i * (8 + factor));
					rx1_spiReader(iqdata);
					int j =0;
					for (j; j< 6; j++){
						hpsdrdata[index + j] = iqdata[j];
					}	
				}
				
				if (nrx == 2) {
					int i =0;
					for (i=0; i< (504 / (8 + factor)); i++) {
						index = 16 + coarse_pointer + (i * (8 + factor));
						rx2_spiReader(iqdata);
						int j =0;
						for (j; j< 6; j++){
							hpsdrdata[index + j + 6] = iqdata[j];
						}	
					}
				}
				
				sem_post(&mutex);
					
			} else {
				int j = 0;
				for (j; j < (504 / (8 + factor)); j++) {
					index = 16 + coarse_pointer + (j * (8 + factor));
					int i =0;
					for (i; i< 8; i++){
						hpsdrdata[index + i] = 0x00;
						if (nrx == 2){
							hpsdrdata[index + i + 6] = 0x00;
						}
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
				if (sampleSpeed ==0)
					usleep(670);  
				if (sampleSpeed == 1)
					usleep(260); 
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

void rx1_spiReader(unsigned char iqdata[]) {
		
	iqdata[0] = (sampleSpeed & 0x03);
	iqdata[1] = (((rando << 6) & 0x40) | ((dither <<5) & 0x20) |  (att & 0x1F));
	iqdata[2] = ((freq >> 24) & 0xFF);
	iqdata[3] = ((freq >> 16) & 0xFF);
	iqdata[4] = ((freq >> 8) & 0xFF);
	iqdata[5] = (freq & 0xFF);
			
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
}

void rx2_spiReader(unsigned char iqdata[]) {
		
	iqdata[0] = (sampleSpeed & 0x03);
	iqdata[1] = (((rando << 6) & 0x40) | ((dither <<5) & 0x20) |  (att & 0x1F));
	iqdata[2] = ((freq2 >> 24) & 0xFF);
	iqdata[3] = ((freq2 >> 16) & 0xFF);
	iqdata[4] = ((freq2 >> 8) & 0xFF);
	iqdata[5] = (freq2 & 0xFF);
			
	spiXfer(rx2_spi_handler, iqdata, iqdata, 6);
}

void *spiWriter(void *arg) {
	
	int lcount =0;
	gettimeofday(&t20, 0);
	
	while(1) {
		
		if (MOX) {
			
			sem_wait(&tx_full);
			sem_wait(&mutex);
			
			gpioWrite(21, 1); ;	// ptt on
			
			if (lcount % 4800 ==0) {
				//set the tx freq.
				tx_iqdata[0] = 0x00;
				tx_iqdata[1] = 0x00;
				tx_iqdata[2] = ((txfreq >> 24) & 0xFF);
				tx_iqdata[3] = ((txfreq >> 16) & 0xFF);
				tx_iqdata[4] = ((txfreq >> 8) & 0xFF);
				tx_iqdata[5] = (txfreq & 0xFF);
							
				spiXfer(rx2_spi_handler, tx_iqdata, tx_iqdata, 6);
			}		
			tx_iqdata[0] = 0;
			tx_iqdata[1] = drive_level / 6.4;  // convert drive level from 0-255 to 0-39 )
			if (prev_drive_level != drive_level) {
				printf("drive level %d - corrected drive level %d \n", drive_level, tx_iqdata[1]);
				prev_drive_level = drive_level; 
			}
			int i = 0;
			for (i; i < 4; i++){			
				tx_iqdata[2 + i] = get_tx_buffer(); //MSB is first in buffer..
			}
			
			spiXfer(rx1_spi_handler, tx_iqdata, tx_iqdata, 6);
			
			sem_post(&mutex);
			sem_post(&tx_empty); 
			
			lcount ++;
			if (lcount == 48000) {
				lcount = 0;
				gettimeofday(&t21, 0);
				float elapsd = timedifference_msec(t20, t21);
				printf("Code tx mode spi executed in %f milliseconds.\n", elapsd);
				gettimeofday(&t20, 0);
			}
		} else if (running==0) usleep(5000000); else usleep(1000000);
	}
}

void put_tx_buffer(unsigned char  value) {
    tx_buffer[fill_tx] = value;    
    fill_tx = (fill_tx + 1) % TX_MAX; 
}

unsigned char get_tx_buffer() {
    int tmp = tx_buffer[use_tx];   
    use_tx = (use_tx + 1) % TX_MAX;  	
    return tmp;
}

//end of source.