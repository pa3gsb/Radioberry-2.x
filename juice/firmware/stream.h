#ifndef __RADIOBERRY_IQ_STREAM_H__
#define  __RADIOBERRY_IQ_STREAM_H__


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "ftd2xx.h"
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>


int init_stream(void);

int read_stream(unsigned char stream[]);

int write_stream(unsigned char stream[]);

int deinit_stream(void);

#endif