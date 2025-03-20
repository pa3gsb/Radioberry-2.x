/*
	This module loads the gateware for the radioberry into the FPGA.
	
	The gateware is present in the radioberry.rbf file.
	
	This file will be read and will be transferred via the usb bus, ft2232 chip into the FPGA.

*/
#ifndef __RADIOBERRY_GATEWARE_LOADER_H__
#define  __RADIOBERRY_GATEWARE_LOADER_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "ftd2xx.h"
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#ifndef _WIN32
#include <sys/time.h>
#endif

#define CHUNK_SIZE 240 

//FPGA Program Pin Information
#define DCLK        0x01 //Pin #1 (DCLK)
#define DATA0       0x02 //Pin #2 (DATA0)
#define NCONFIG     0x04 //Pin #3 (NCONFIG)
#define NSTATUS     0x08 //Pin #4 (NSTATUS)
#define NCONF_DONE  0x10 //Pin #5 (NCONF_DONE)

#define SET_DCLK(s,b)    (s |= (b << 0))
#define SET_DATA0(s,b)   (s |= (b << 1))
#define SET_NCONFIG(s,b) (s |= (b << 2))

#define CLEAR_DCLK(s)    (s &= ~(1 << 0))
#define CLEAR_DATA0(s)   (s &= ~(1 << 1))
#define CLEAR_NCONFIG(s) (s &= ~(1 << 2))

int load_gateware_image_into_fpga(void);

unsigned long getFirmwareVersion(void);

#endif