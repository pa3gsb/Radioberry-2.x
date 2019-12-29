/* 
* Copyright (C)
* 2017, 2018, 2019, 2020 - Johan Maas, PA3GSB
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
* This programs is the c part of the firmware for the RadioBerry. 
*
*	****  RUNNING 'ONLY' AT  RPI-4.  ****
*
* By using this program you have the possibility to connect to SDR programs (using UDP) like:
*	- pihpsdr (including TCP)
*	- Quisk
*	- PSDR
*	- SPARK
*	- SDRConsole
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
#include "radioberry.h"
#include "radioberry-rpi.h"

int main(int argc, char **argv)
{
	printIntroScreen();
	
	unsigned int rev = gpioHardwareRevision();  
	//A03111 hex
	int rpi4 = (rev >= 10498321) ? 1 : 0;
	if (!rpi4) {
		fprintf(stderr, "You are not running a RPI-4; closing radioberry firmware.");
		return -1;
	}
	
	if (initRadioberry() < 0){
		fprintf(stderr,"Radioberry; could not be initialized. \n");
		exit(-1);
	}
	gettimeofday(&t20, 0);
	runRadioberry();
	closeRadioberry();
}

int initRadioberry() {
	sem_init(&mutex, 0, 1);	
	sem_init(&tx_empty, 0, TX_MAX); 
    sem_init(&tx_full, 0, 0);   

	memset(commands,0,256); // initialise the commands from SDR program.	
	
	if (gpioInitialise() < 0) {
		fprintf(stderr,"Radioberry;  gpio could not be initialized. \n");
		exit(-1);
	} 
	
	rx1_spi_handler = spiOpen(0, 9000000, 49155);  //channel 0
	if (rx1_spi_handler < 0) {
		fprintf(stderr,"radioberry_protocol: spi bus rx1 could not be initialized. \n");
		exit(-1);
	}

	fprintf(stderr, "Radioberry, Initialisation succesfully executed.\n");
		
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
			active = 1;
			sendPacket();
		} else {active = 0; usleep(20000); }
	}
}

void *packetreader(void *arg) {
	
	init_RPI_GPIO();
	
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
			int recvlen = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&remaddr, &addrlen);
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
			unsigned char reply[22] = {0xEF, 0xFE, 0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, FIRMWARE_VERSION, HERMESLITE, 0, 0, 0, 0, 0, 0, 0, 0, NR, 0, MINOR_VERSION };
			memcpy(broadcastReply, reply, 22);
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
		case 0x0304feef:
			fprintf(stderr, "Start Port %d \n", ntohs(remaddr.sin_port));
			running = 1;
			if (sock_TCP_Client > -1)
				fprintf(stderr, "SDR Program sends TCP Start command \n");
			else
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


#define assign_change(a,b,c) if ((a) != b) { b = (a); fprintf(stderr, "%20s= %08lx (%10ld)\n", c, (long) b, (long) b ); }
void processPacket(char* buffer)
{	
	//nrx for internal use required.
	if ((buffer[ 11] & 0xFE) == 0x00) {
		assign_change((((buffer[ 15] & 0x38) >> 3) + 1), nrx, "Receivers");
	}
	if ((buffer[523] & 0xFE)  == 0x00) {
		assign_change((((buffer[527] & 0x38) >> 3) + 1), nrx, "Receivers");
	}
	
	MOX = ((buffer[11] & 0x01)==0x01) ? 1:0;
	
	command = buffer[ 11];
	command_data=((buffer[12]&0xFF)<<24)+((buffer[13]&0xFF)<<16)+((buffer[14]&0xFF)<<8)+(buffer[15]&0xFF);
	if (commands[command] != command_data) {
		commands[command] = command_data;
		spi_send_control(command);
	}
	command = buffer[ 523];
	command_data =((buffer[524]&0xFF)<<24)+((buffer[525]&0xFF)<<16)+((buffer[526]&0xFF)<<8)+(buffer[527]&0xFF);
	if (commands[command] != command_data) {
		// make logging options for reading the command data...
		commands[command] = command_data;
		spi_send_control(command);
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

		while ((((*rpi_read_io) >> 25) & 1) == 0) { usleep(rb_sleep); }//wait for enough samples 
		int factor = (nrx - 1) * 6;
		iqs =0; 
		for (int frame = 0; frame < 2; frame++) {
			int coarse_pointer = frame * 512; // 512 bytes total in each frame
			for (int i=0; i< (504 / (8 + factor)); i++) {
				int index = 16 + coarse_pointer + (i * (8 + factor));
				for (int r=0; r < nrx; r++) {	
						rx_reader(iqdata);
						memcpy(hpsdrdata + index + (r * 6), iqdata, 6);
				}
			}
		}
}

void rx_reader(unsigned char iqdata[]){
	uint32_t value = 0;
	
	iqs++;
	uint32_t lastid  =0;
		
	for (int i = 0; i < 6 ; i++) {
		*rpi_set_io_high = (1<<RPI_RX_CLK);
		value = *rpi_read_io;
		iqdata[i] =  (((value >> 16) & 1) << 7);
		iqdata[i] |=  (((value >> 19) & 1) << 6);
		iqdata[i] |=  (((value >> 20) & 1) << 5);
		iqdata[i] |=  (((value >> 21) & 1) << 4);
		
		
		*rpi_set_io_low = (1<<RPI_RX_CLK);
		value = *rpi_read_io;
		iqdata[i] |=  (((value >> 16) & 1) << 3);
		iqdata[i] |=  (((value >> 19) & 1) << 2);
		iqdata[i] |=  (((value >> 20) & 1) << 1);
		iqdata[i] |=  (((value >> 21) & 1));

		lastid |= (((value >> 23) & 1) << (i));
					
	}		

	//fprintf(stderr, "samples %x-%x-%x-%x-%x-%x lastid %x \n", iqdata[0] , iqdata[1] ,iqdata[2] ,iqdata[3] ,iqdata[4] ,iqdata[5], lastid );
	
	// sync with iq samples from gateware... 
	
	// needs improvement. after start or stop/start seq the iq samples seems not in the right order.
	// the gateware provides in a last indicator. Assuming sequence I0Q0I1Q1 the Q1 is containing this last indicator.
	// the given sequence must be kept!! But after a start or restart the sequence from the gateware is not quaranteed.
	// in the gateware i need to sync the reading... reading always a block of samples for all rx slices..
	// if gateware is taking the control... one gpio/fpga pin comes free!
	if ( (iqs % nrx== 0) && (lastid !=0x07)  ) {
		*rpi_set_io_high = (1<<RPI_RX_CLK); 
		*rpi_set_io_low = (1<<RPI_RX_CLK);
	}
}

void spi_send_control(unsigned char command) {
	
	unsigned char data[6];
	uint32_t command_data = commands[command];
	
	
	if (command & 0x80) fprintf(stderr, "ack = 1 \n");
	data[0] = (run & 0xFF);		//MSB
	data[1] = (command & 0xFF);
	data[2] = ((command_data >> 24) & 0xFF);
	data[3] = ((command_data >> 16) & 0xFF);
	data[4] = ((command_data >> 8) & 0xFF);
	data[5] = (command_data & 0xFF);
		
	sem_wait(&mutex);		
	spiXfer(rx1_spi_handler, data, data, 6);
	sem_post(&mutex);
	
	//int fifolength = ((data[4] & 0x03) << 8) + (data[5] & 0xFF);
	//fprintf(stderr, "Fifo length %d.\n", fifolength);
	//fprintf(stderr, "response address %x data %x-%x-%x-%x \n", data[1], data[2], data[3], data[4], data[5]);
}

void *spiWriter(void *arg) {
	
	gettimeofday(&t20, 0);
	
	while(1) {
		
		sem_wait(&tx_full);
  
		int i = 0;
		for (i; i < 8; i++){			
			tx_iqdata[i] = get_tx_buffer(); //EER first 4 bytes followed by 4 bytes TX IQ.
		}
		//first setup without EER
		if (MOX) {
			for (int i = 4; i < 8 ; i++) {
				
				if ( (tx_iqdata[i] & 0x80) == 0x80) *rpi_set_io_high = (1<<17); else *rpi_set_io_low = (1<<17);
				if ( (tx_iqdata[i] & 0x40) == 0x40) *rpi_set_io_high = (1<<5); else *rpi_set_io_low = (1<<5);
				if ( (tx_iqdata[i] & 0x20) == 0x20) *rpi_set_io_high = (1<<18); else *rpi_set_io_low = (1<<18);
				if ( (tx_iqdata[i] & 0x10) == 0x10) *rpi_set_io_high = (1<<12); else *rpi_set_io_low = (1<<12);
				*rpi_set_io_high = (1<<RPI_TX_CLK);
				
				if ( (tx_iqdata[i] & 0x08) == 0x08) *rpi_set_io_high = (1<<17); else *rpi_set_io_low = (1<<17);
				if ( (tx_iqdata[i] & 0x04) == 0x04) *rpi_set_io_high = (1<<5); else *rpi_set_io_low = (1<<5);
				if ( (tx_iqdata[i] & 0x02) == 0x02) *rpi_set_io_high = (1<<18); else *rpi_set_io_low = (1<<18);
				if ( (tx_iqdata[i] & 0x01) == 0x01) *rpi_set_io_high = (1<<12); else *rpi_set_io_low = (1<<12);
				*rpi_set_io_low = (1<<RPI_TX_CLK);
			}
		}	

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

void put_tx_buffer(unsigned char  value) {
    tx_buffer[fill_tx] = value;    
    fill_tx = (fill_tx + 1) % TX_MAX_BUFFER; 
}

unsigned char get_tx_buffer() {
    unsigned char tmp = tx_buffer[use_tx];   
    use_tx = (use_tx + 1) % TX_MAX_BUFFER;  	
    return tmp;
}

//end of source.