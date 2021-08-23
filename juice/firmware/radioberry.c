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
* This programs is the c part of the firmware for RadioBerry Juice. 
*
* The belonging gateware can be found at: https://github.com/softerhardware/Hermes-Lite2
*
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

#include "radioberry.h"
#include "gateware.h"
#include "stream.h"
#include "register.h"

#ifdef _WIN32
	#define FIRMWAREVERSION "W-J-2021-08-01"
#else
	#define FIRMWAREVERSION "L-J-2021-08-01"
#endif

void printIntroScreen() {
	fprintf(stderr,"\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr, "\t\t\tRadioberry V2.0\n\n\n");
	fprintf(stderr,	"\tSupports 6 receivers and 1 transmitter.\n\n");
	fprintf(stderr, "\tBuild version: %s\n\n", FIRMWAREVERSION);
	fprintf(stderr, "\tHave fun Johan PA3GSB\n\n");
	fprintf(stderr, "\tReport requests or bugs to <pa3gsb@gmail.com>.\n");
	fprintf(stderr, "====================================================================\n");
	fprintf(stderr, "====================================================================\n");
}

void handle_sigint(int sig) 
{ 
	if (running) fprintf(stderr, "  SDR program is still running; please stop SDR first.\n");
	closerb = 1;
}

int load_radioberry_gateware() {
	
	int ret = 0;
	
	time_t gstartTime = clock();

	if (ret = load_gateware_image_into_fpga() < 0) { return ret; };

	time_t gstopTime = clock();
	double gsecondsElapsed = (double)(gstopTime - gstartTime) / CLOCKS_PER_SEC;
	
	fprintf(stderr, "Radioberry gateware loaded taking %0.1f seconds.\r\n", gsecondsElapsed);
	
	return ret;
}

static void *rb_register_thread(void *arg) {
	sprintf(gatewareversion,"%d.%d", gateware_major_version, gateware_minor_version);
	sprintf(firmwareversion,"%s", FIRMWAREVERSION);
	//major-minor-build of ftdi driver
	sprintf(driverversion,"%02x.%02x.%02x", ((driver_version >>16) & 0xFF), ((driver_version >>8) & 0xFF), (driver_version & 0xFF));
	gateware_fpga_type == 0 ? sprintf(fpgatype,"%s", "-") : gateware_fpga_type == 1 ? sprintf(fpgatype,"%s", "CL016") : sprintf(fpgatype,"%s", "CL025");
	registerRadioberry();
	return NULL;
}

void start_rb_register_thread() {
	pthread_t pid1; 
	pthread_create(&pid1, NULL, rb_register_thread, NULL);
}

void getStreamAndSendPacket() {
	if (read_stream(hpsdrdata) < 0) return; // fetching 1032 bytes of ethernet packet.
	if (sock_TCP_Client >= 0) {
		if (sendto(sock_TCP_Client, (const char*) hpsdrdata, sizeof(hpsdrdata), 0, NULL, 0) != 1032) fprintf(stderr, "TCP send error");
	} else {
		if (sendto(fd, (const char*) hpsdrdata, sizeof(hpsdrdata), 0, (struct sockaddr *)&remaddr, addrlen) != 1032) fprintf(stderr, "UDP send error");
	}
}

void runRadioberry() {
	fprintf(stderr, "Radioberry, Starting packet tx part. \n");
	while(!(running==0 && closerb == 1)) {
		if (running) {
			active = 1;
			getStreamAndSendPacket();
		} else {active = 0; usleep(20000); }
	}
}

void *packetreader(void *arg) {
	int size, bytes_read, bytes_left;
	unsigned char buffer[1032];
	
	memset(buffer, 0, 1032);
	write_stream(buffer);
	write_stream(buffer);
	
	uint32_t *code0 = (uint32_t *) buffer; 
	fprintf(stderr, "Radioberry, Starting packet rx part. \n");
	while(1) {
		
		if (sock_TCP_Client >= 0) {
			// handle TCP protocol.
			bytes_read=0;
			bytes_left=1032;
			while (bytes_left > 0) {
				size = recvfrom(sock_TCP_Client, (char*) buffer+bytes_read, (size_t) bytes_left, 0, NULL, 0);
				if (size < 0 && errno == EAGAIN) continue;
				if (size < 0) break;
				bytes_read += size;
				bytes_left -= size;
			}
			if (bytes_read == 1032) handlePacket((unsigned char*) buffer); //else fprintf(stderr, "tcp packet received; wrong length %d \n", bytes_read);
		} 
		else {
			// handle UDP protocol.
			int recvlen = recvfrom(fd, (char*) buffer, sizeof(buffer), 0, (struct sockaddr *)&remaddr, &addrlen);
			if (recvlen > 0) { udp_retries = 0; handlePacket((unsigned char*) buffer); } else udp_retries++;
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

void handlePacket(unsigned char* buffer){
	uint32_t code;
	memcpy(&code, buffer, 4);
	switch (code)
	{
		default:
			{
			fprintf(stderr, "Received packages not for me! \n");
			}
			break;
		case 0x0002feef:
			{
			fprintf(stderr, "Discovery packet received \n");
			fprintf(stderr,"SDR Program IP-address %s  \n", inet_ntoa(remaddr.sin_addr)); 
			fprintf(stderr, "Discovery Port %d \n", ntohs(remaddr.sin_port));
			memset(broadcastReply, 0, 60);
			unsigned char reply[22] = {0xEF, 0xFE, 0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, gateware_major_version, HERMESLITE, 0, 0, 0, 0, 0, 0, 0, 0, NR, 0, gateware_minor_version };
			memcpy(broadcastReply, reply, 22);
			if (sock_TCP_Client > -1) {
				send(sock_TCP_Client, (const char*) broadcastReply, 60, 0);
				close(sock_TCP_Client);
				sock_TCP_Client = -1;
				}
			else if (sendto(fd, (const char*) broadcastReply, sizeof(broadcastReply), 0, (struct sockaddr *)&remaddr, addrlen) < 0) fprintf(stderr, "broadcast reply error");
			}
			break;
		case 0x0004feef:
			{
			fprintf(stderr, "SDR Program sends Stop command \n");
			running = 0;
			while (active) usleep(1000);
			if (sock_TCP_Client > -1)
			{
				close(sock_TCP_Client);
				sock_TCP_Client = -1;
				fprintf(stderr, "SDR Program sends TCP Stop command \n");
			} else fprintf(stderr, "SDR Program sends UDP Stop command \n"); 
			}
			write_stream(buffer);
			break;
		case 0x0104feef:
		case 0x0304feef:
			{
			fprintf(stderr, "Start Port %d \n", ntohs(remaddr.sin_port));
			running = 1;
			if (sock_TCP_Client > -1)
				fprintf(stderr, "SDR Program sends TCP Start command \n");
			else
				fprintf(stderr, "SDR Program sends UDP Start command \n");
			}
			write_stream(buffer);
			break;
		case 0x1104feef: 
			{
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
			}
			write_stream(buffer);
			break;
		case 0x0201feef:
			{
			write_stream(buffer);
			}
			break;		
	}
}

int main(int argc, char **argv)
{	
	printIntroScreen();

	if (load_radioberry_gateware() < 0) {
		fprintf(stderr,"Radioberry; loading radioberry  \n");
		exit(-1);
	}
	
	driver_version = getFirmwareVersion();
	//fprintf(stderr, "%06x\n", driver_version);
	
	int ret;
	if (ret = init_stream() < 0) { return ret; }	
	
#ifdef _WIN32	
	WSADATA wsa;
	
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}
#endif
	
	pthread_t pid1; 
	pthread_create(&pid1, NULL, packetreader, NULL); 
	
	/* create a UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot create socket %d\n", fd);
		perror("cannot create socket\n");
		return 0;
	}
	struct timeval timeout;      
    timeout.tv_sec = 0;
    timeout.tv_usec = TIMEOUT_MS;
	int wtimeout = TIMEOUT_MS;  

#ifdef _WIN32
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&wtimeout,sizeof (int)) < 0)
		perror("setsockopt failed\n");	
#else
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0)
		perror("setsockopt failed\n");
		
	int optval = 7; // high priority.
	if (setsockopt(fd, SOL_SOCKET, SO_PRIORITY, &optval, sizeof(optval))<0) {
        perror("UDP socket: SO_PRIORITY");
    }
#endif
	
	/* bind the socket to any valid IP address and a specific port */
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(SERVICE_PORT);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}
	
	if ((sock_TCP_Server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP )) < 0)
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
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVTIMEO, (const char *)&wtimeout, sizeof(wtimeout)); //added
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
	
#ifndef _WIN32
	//int optval = 7; // high priority.
	if (setsockopt(sock_TCP_Server, SOL_SOCKET, SO_PRIORITY, &optval, sizeof(optval))<0) {
        perror("sock_TCP_Server socket: SO_PRIORITY");
    }
#endif
	
	if (bind(sock_TCP_Server, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
	{
		perror("bind tcp");
		return -1;
	}
	listen(sock_TCP_Server, 1024);
	
#ifdef _WIN32
	int addrlen;
	unsigned long mode;
#else
	int fl;
	socklen_t addrlen;
#endif
	
#ifdef _WIN32
	mode = 1;
	ioctlsocket(sock_TCP_Server, FIONBIO, &mode);
#else
	int flags = fcntl(sock_TCP_Server, F_GETFL, 0);
    fcntl(sock_TCP_Server, F_SETFL, flags | O_NONBLOCK);
#endif

	start_rb_register_thread();
	
	signal(SIGINT, handle_sigint);
	
	runRadioberry();
	
	closeRadioberry();
	
	return 0;
}

int closeRadioberry() {
	fprintf(stderr, "close radioberry\n");
	
	deinit_stream();
	
	if (fd_rb != 0) close(fd_rb);
	if (sock_TCP_Client >= 0) close(sock_TCP_Client);
	if (sock_TCP_Server >= 0) close(sock_TCP_Server);
#ifdef _WIN32
	closesocket(fd);
	WSACleanup();
#endif

	return 0;
}
//end of source.