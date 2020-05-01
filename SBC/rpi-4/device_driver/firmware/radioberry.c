/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>


* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* This programs is the c part of the firmware for the RadioBerry. 
*
* The belonging gateware can be found at: https://github.com/softerhardware/Hermes-Lite2
*
*	****  Device Driver version. 	****
*
* By using this program you have the possibility to connect to SDR programs like:
*	- pihpsdr (including TCP)
*	- Quisk
*	- PSDR
*	- SPARK
*	- SDRConsole
*
*	Using the 'HPSDR UDP protocol'; also called protocol-1
*
*
*	http://www.pa3gsb.nl
*	  
*	Johan PA3GSB
*
*/

#define NR 0x04
#define HERMESLITE  0x06

#include "radioberry_ioctl.h"
#include "radioberry.h"
#include "filters.h"

int main(int argc, char **argv)
{	
	printIntroScreen();
	if (initRadioberry() < 0){
		fprintf(stderr,"Radioberry; could not be initialized. \n");
		exit(-1);
	}
	signal(SIGINT, handle_sigint);
	runRadioberry();
	closeRadioberry();
}

int initRadioberry() {
	sem_init(&mutex, 0, 1);	
	sem_init(&tx_empty, 0, TX_MAX); 
    sem_init(&tx_full, 0, 0); 

	gettimeofday(&t10, 0);	
	gettimeofday(&t20, 0);	

	memset(commands,0,256); // initialise the commands.	
	
	if((fd_rb = open("/dev/radioberry", O_RDWR)) < 0){
		perror("Failed to open /dev/radioberry");
		exit(-1);
	}
	
	//rb_info.rb_command = 0x01; 
	rb_info.command_data = 0x0;
	
	//required to retrieve gateware information.
	if (ioctl(fd_rb, RADIOBERRY_IOC_COMMAND, &rb_info) == -1) {
		fprintf(stderr, "RADIOBERRY_IOC_COMMAND Error.");
		exit(-1);		
	} 
	gateware_major_version = rb_info.major;
	gateware_minor_version = rb_info.minor;
	fprintf(stderr, "Radioberry gateware version %d-%d.\n", rb_info.major, rb_info.minor);
	

	fprintf(stderr, "Radioberry firmware initialisation succesfully executed.\n");

	//***********************************************
	//       Filters switching initialization
	//***********************************************
	initFilters();
	//***********************************************	

	pthread_t pid1, pid2; 
	pthread_create(&pid1, NULL, packetreader, NULL); 
	pthread_create(&pid2, NULL, txWriter, NULL);

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
	int yes = 1;
	setsockopt(sock_TCP_Server, IPPROTO_TCP, TCP_MAXSEG, (const char *)&tcpmaxseg, sizeof(int));
	int sndbufsize = 65535;
	int rcvbufsize = 65535;
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_SNDBUF, (const char *)&sndbufsize, sizeof(int));
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVBUF, (const char *)&rcvbufsize, sizeof(int));
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeout)); //added
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_REUSEADDR, (void *)&yes, sizeof(yes));

	if (bind(sock_TCP_Server, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
	{
		perror("bind tcp");
		return -1;
	}
	listen(sock_TCP_Server, 1024);
	int flags = fcntl(sock_TCP_Server, F_GETFL, 0);
    fcntl(sock_TCP_Server, F_SETFL, flags | O_NONBLOCK);
}

int closeRadioberry() {
	if (fd_rb != 0) close(fd_rb);
	if (sock_TCP_Client >= 0) close(sock_TCP_Client);
	if (sock_TCP_Server >= 0) close(sock_TCP_Server);
	
	return 0;
}

void runRadioberry() {
	fprintf(stderr, "Radioberry, Starting packet tx part. \n");
	while(!(running==0 && closerb == 1)) {
		if (running) {
			active = 1;
			sendPacket();
		} else {active = 0; usleep(20000); }
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
			unsigned char reply[22] = {0xEF, 0xFE, 0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, gateware_major_version, HERMESLITE, 0, 0, 0, 0, 0, 0, 0, 0, NR, 0, gateware_minor_version };
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
				send_control(0);
				fprintf(stderr, "SDR Program sends TCP Start command \n");
			}	
			break;
		case 0x0201feef:
			processPacket(buffer);
			break;		
	}
}

#define assign_change(a,b,c) if ((a) != b) { b = (a); fprintf(stderr, "%20s= %08lx (%10ld)\n", c, (long) b, (long) b ); }

void handleCommand(int base_index, char* buffer) {
	command = buffer[base_index];
	command_data=((buffer[base_index+1]&0xFF)<<24)+((buffer[base_index+2]&0xFF)<<16)+((buffer[base_index+3]&0xFF)<<8)+(buffer[base_index+4]&0xFF);
	if ((commands[command] != command_data) | (save_mox != MOX)) {
		commands[command] = command_data;
		
		if ((command & 0x1E) == 0x1E) CWX = (command_data & 0x01000000) ? 1:0;
		
		send_control(command);
	}
}

void handleCommands(char* buffer) {
	handleCommand(11, buffer);
	handleCommand(523, buffer);
	save_mox = MOX;
}

void processPacket(char* buffer)
{	
	MOX = ((buffer[11] & 0x01)==0x01) ? 1:0;
	
	handleCommands(buffer);
		
	int frame = 0;
	for (frame; frame < 2; frame++)
	{
		int coarse_pointer = frame * 512 + 8;
		int j = 8;
		for (j; j < 512; j += 8)
		{
			int k = coarse_pointer + j;
			if (MOX || CWX) {
				sem_wait(&tx_empty);
				int i = 0;
				for (i; i < 8; i++){
					put_tx_buffer(buffer[k + i]);	
				}
				sem_post(&tx_full);
			}
		}
	}
	
	//nrx for internal use required.
	if ((buffer[ 11] & 0xFE) == 0x00) {
		assign_change((((buffer[ 15] & 0x38) >> 3) + 1), nrx, "Receivers");
	}
	if ((buffer[523] & 0xFE)  == 0x00) {
		assign_change((((buffer[527] & 0x38) >> 3) + 1), nrx, "Receivers");
	}
	//**************************************************
	//         Handling filter change
	//**************************************************
	handleFilters(buffer, CWX);
	//**************************************************
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

		lnrx = nrx;

		int factor = (lnrx - 1) * 6;
		for (int frame = 0; frame < 2; frame++) {
			int coarse_pointer = frame * 512; // 512 bytes total in each frame
			
			int nr_samples = (nrx == 1)? 63 : (nrx == 2)? 72: (nrx ==3)? 75: 76;
			read(fd_rb , rx_buffer , nr_samples);
			rb_sample = 0;
			for (int i=0; i< (504 / (8 + factor)); i++) {
				int index = 16 + coarse_pointer + (i * (8 + factor));
				//NR must be read from gateware.
				for (int r=0; r < MIN(lnrx, NR); r++) {	
					memcpy(hpsdrdata + index + (r * 6), rx_buffer + rb_sample, 6);
					rb_sample+=6;						
				}
			}
		}
}

void send_control(unsigned char command) {

	unsigned char data[6];
	uint32_t command_data = commands[command];
	
	rb_info.rb_command = (((CWX << 1) & 0x02) | (running & 0x01));
	rb_info.command = command;
	rb_info.command_data = command_data;
	
	fprintf(stderr, "RB-Command = %02X Command = %02X  command_data = %08X\n", rb_info.rb_command, command, command_data);
	
	if (ioctl(fd_rb, RADIOBERRY_IOC_COMMAND, &rb_info) == -1) {
		fprintf(stderr, "Could not sent commando to radioberry device.");
	}
}

void *txWriter(void *arg) {
	
	gettimeofday(&t20, 0);
	
	while(1) {
		
		sem_wait(&tx_full);
  
		int i = 0;
		for (i; i < 4; i++){
			get_tx_buffer(); //EER first 4 bytes followed by 4 bytes TX IQ.
		}
		for (i=0; i < 4; i++){
			tx_iqdata[i] = get_tx_buffer(); //EER first 4 bytes followed by 4 bytes TX IQ.
		}

		//first setup without EER
		if (MOX || CWX) {
			write(fd_rb , tx_iqdata , sizeof(tx_iqdata));
		}	
	
		sem_post(&tx_empty); 
		
		tx_count ++;
		if (tx_count == 48000) {
			tx_count = 0;
			gettimeofday(&t21, 0);
			float elapsed = timedifference_msec(t20, t21);
			fprintf(stderr, "Code tx write executed in %f milliseconds.\n", elapsed);
			//fprintf(stderr, "tx_iqdata = %02X - %02X - %02X - %02X\n", tx_iqdata[4], tx_iqdata[5], tx_iqdata[6], tx_iqdata[7]);
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