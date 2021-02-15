#ifndef __RADIOBERRY_MEASURE_H__
#define __RADIOBERRY_MEASURE_H__

/**
	MAX11613 4 channel ADC.
	
	For the radioberry preAmp this devices does measure the following analog signals:
	
	- Current of the radioberry preamp fets.
	- Temperature of the radioberry preamp fets.
	
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


#define ADDR_MEAS 0x34


int i2c_measure_module_active;

int fd_i2c_measure;
int i2c_measure_handler;

void openI2C_measure(void);
void read_I2C_measure(int *current, int *temperature);
void close_I2C_measure(void);

#endif