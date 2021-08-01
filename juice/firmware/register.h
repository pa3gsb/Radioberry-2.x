
#ifndef __RADIOBERRY_REGISTER_H__
#define __RADIOBERRY_REGISTER_H__

#ifdef _WIN32
#define TCP_MAXSEG	4
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib,"ws2_32.lib") //Winsock Library

#else
#include <unistd.h>
#include <linux/if.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

#include <string.h>

typedef struct _PROPERTY PROPERTY;
struct _PROPERTY {
    char* name;
    char* value;
	PROPERTY* next_property;
};

extern char macaddress[24];
extern char* radiocall; 
extern char* radiolocator;
extern char gatewareversion[16];
extern char driverversion[16];
extern char firmwareversion[16]; 
extern char fpgatype[16];

int getWindowsMacAddress(void);
int getMacAddress(void);
void loadProperties(char* filename);
char* getProperty(char* name);
void loadRadioberryProps(void);
ssize_t process_http(int sockfd, char *host, char *page, char *poststr);
int postRadioberryConfiguration(void);

extern void registerRadioberry(void);

#endif