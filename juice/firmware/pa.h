#ifndef __RADIOBERRY_PA_H__
#define __RADIOBERRY_PA_H__

/**

RB2AMPController (Radioberry V2.0 Amplifier Controller)
https://github.com/pa3gsb/RB2AMPController

The AMP controller contains a TCPIP server. This firmware
talks to the controller server via TCPIP client.

The information in the openhpsdr protocol-1 is used to control the amplifier.
So if you change the frequency in your favourite SDR program the LPF filter
in the AMP will be set. Also when going into transmit; the AMP will
make use of a sequencer and will switch the relais.

Information of the AMP will be placed into the protocol and will be 
presented in the SDR program user interface to the radio amateur.
Information here is the temperature of the AMP, the forward and reverse power.

RB2AMPController-Protocol

REQUEST (to AMP Controller server)

(byte index is 0 based)
indentification bytes[0-3]:	0x00 0x00 0xef 0xfe 
byte 4:  	off 	0x00 
			standby 0x0f  
			active  0xff  
byte 5:  	PTT  	0x00 	PA off.
					0xFF  	PA on.
byte 6:  	lpf  	0x00 	no LPF filter.
					0x80 	160m
					0x40 	80m
					0x20 	60m
					0x10	40m
					0x08	20m
					0x04	10m
				
Total 20 bytes (so some spare bytes)


RESPONSE (from AMP Controller server)

identification 	bytes[0-3]:	0x00 0x00 0xef 0xfe
byte 4:			versie pa firmware  (1/10 => 0.1)  divide by 10.
byte 5, 6		byte 5 0x00 temp measure is ok ; 0xFF temp measure error for byte 6 temp in celsius. (no fractions) 
byte 7, 8		forwrd pwr
byte 9,10		reverse pwr
byte 11,12		u   (not implemented) 
byte 13,14		i	(not implemented) 
byte 19			error  

Total 20 bytes (so some spare bytes)

*/

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library
#else
#include <arpa/inet.h>
#include <stdbool.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <semaphore.h>

#define TIMEOUT_MS      100


extern int pa_version;
extern bool pa_temp_ok;
extern uint16_t pa_temp;
extern uint16_t pa_fwd_dac;
extern uint16_t pa_rev_dac;
extern int pa_err;

void initAmplifier(void);
int connectToAmplifier(void);
int isAmplifierConnected(void);
void disconnectAmplifier(void);
void setAmplifierInfo(void);
void setAmplifierInfoBuffer(unsigned char* buffer); 
void exhangeAmplifierInfo(void); 

extern int pa_connected;

#endif

