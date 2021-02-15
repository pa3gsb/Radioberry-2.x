#ifndef __RADIOBERRY_BIAS_H__
#define __RADIOBERRY_BIAS_H__

/**
	MCP4662 Dual digital potentiometer.
	
	This device is used to set both bias settings for the Radioberry preAmp.
	
	The setting can be made permanent (non volatile) by saving the bias values in the EEPROM of the device.
	
	The bias is a specific setting stored and determined specific per preAmp board. 

	The setting can be done by QUISK or SparkSDR using the extended Protocol-1 for Hermes Lite
	https://github.com/softerhardware/Hermes-Lite2/wiki/Protocol
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>


#define ADDR_BIAS 0x2C

int fd_i2c_bias;
int i2c_bias_handler;

void openI2C_bias(void);
void write_I2C_bias(uint8_t control, uint8_t data);
void close_I2C_bias(void);

#endif