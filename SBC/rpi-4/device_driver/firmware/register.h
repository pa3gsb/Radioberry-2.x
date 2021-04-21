
#ifndef __RADIOBERRY_REGISTER_H__
#define __RADIOBERRY_REGISTER_H__


#include <linux/if.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>


typedef struct _PROPERTY PROPERTY;
struct _PROPERTY {
    char* name;
    char* value;
	PROPERTY* next_property;
};

char macaddress[24];
char* radiocall; 
char* radiolocator;
char gatewareversion[16];
char driverversion[16];
char firmwareversion[16]; 
char fpgatype[16];

int getMacAddress(void);
void loadProperties(char* filename);
char* getProperty(char* name);
void loadRadioberryProps(void);
ssize_t process_http(int sockfd, char *host, char *page, char *poststr);
int postRadioberryConfiguration(void);


extern void registerRadioberry(void);

#endif