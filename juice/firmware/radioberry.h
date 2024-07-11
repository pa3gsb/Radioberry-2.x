#ifndef __RADIOBERRY_H__
#define __RADIOBERRY_H__

#ifdef _WIN32
#define TCP_MAXSEG	4
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#else
#include <linux/if.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/utsname.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>



#define TIMEOUT_MS      100
#define SERVICE_PORT	1024
#define NR 0x06
#define HERMESLITE  0x06


int sys_temp = 0; //rpi-temperature.
int rpiLinux = 0;

char macaddress[24];
char* radiocall; 
char* radiolocator;
char gatewareversion[16];
char driverversion[16];
char firmwareversion[16]; 
char fpgatype[16];

int fd_rb;

unsigned char run = 0;
int closerb = 0;
int initRadioberry();
void runRadioberry(void);
int closeRadioberry();
void handlePacket(unsigned char* buffer);
void *packetreader(void *arg);
int sock_TCP_Server = -1;
int sock_TCP_Client = -1;
int udp_retries=0;
int active = 0;
static volatile int keepRunning = 1;
unsigned char gateware_major_version = 73;
unsigned char gateware_minor_version = 0;
unsigned long driver_version = 0x00020406;
int gateware_fpga_type = 2;
unsigned char hpsdrdata[1032];
unsigned char broadcastReply[60];
void write_rb_stream(unsigned char* buffer);
    
int running = 0;
int fd;									/* our socket */

struct sockaddr_in myaddr;				/* our address */
struct sockaddr_in remaddr;				/* remote address */

socklen_t addrlen = sizeof(remaddr);	/* length of addresses */


#endif