/* 
* Copyright (C)
* 2017, 2018, 2019 - Johan Maas, PA3GSB
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
*	Measering data throughput....
* Constraint:
* == Zorg hierbij dat de fifo altijd voldoende vulling heeft!!==
*
* Lezen van 192000 samples (6 bytes * 192000) voor 384K spectrum
*
* -> meten van de tijd;
* -> controle op volgnr;
* -> zonder controle opvolgnr;
* -> vullen buffer...
*
* ==> zonder pull up voor clock pin
* ==> Met pull up voor clock pin
* ==> Vervolgens met weerstand van 1K van 3V3 naar clock pin 
*
*	http://www.pa3gsb.nl
*	  
*	Johan PA3GSB
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>


#include <pigpio.h>
#include "performance.h"

volatile uint32_t *eread_reg=NULL;


// ---- GPIO specific defines
#define GPIO_REGISTER_BASE 0x200000
#define GPIO_SET_OFFSET 0x1C
#define GPIO_CLR_OFFSET 0x28
#define GPIO_LEV0_OFFSET 0x34
#define PHYSICAL_GPIO_BUS (0x7E000000 + GPIO_REGISTER_BASE)

#define PAGE_SIZE 4096
#define BCM2708_PI1_PERI_BASE  0x20000000
#define BCM2709_PI2_PERI_BASE  0x3F000000
#define BCM2711_PI4_PERI_BASE  0xFE000000

#define PERI_BASE BCM2711_PI4_PERI_BASE

#define TOGGLE_GPIO 6 // pin 22

// Return a pointer to a periphery subsystem register.
static void *mmap_bcm_register(off_t register_offset) {
  const off_t base = PERI_BASE;

  int mem_fd;
  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
    perror("can't open /dev/mem: ");
    fprintf(stderr, "You need to run this as root!\n");
    return NULL;
  }

  uint32_t *result =
    (uint32_t*) mmap(NULL,                  // Any adddress in our space will do
                     PAGE_SIZE,
                     PROT_READ|PROT_WRITE,  // Enable r/w on GPIO registers.
                     MAP_SHARED,
                     mem_fd,                // File to map
                     base + register_offset // Offset to bcm register
                     );
  close(mem_fd);

  if (result == MAP_FAILED) {
    fprintf(stderr, "mmap error %p\n", result);
    return NULL;
  }
  return result;
}

void initialize_gpio_for_output(volatile uint32_t *gpio_registerset, int bit) {
  *(gpio_registerset+(bit/10)) &= ~(7<<((bit%10)*3));  // prepare: set as input
  *(gpio_registerset+(bit/10)) |=  (1<<((bit%10)*3));  // set as output.
}

void initialize_gpio_for_input(volatile uint32_t *gpio_registerset, int bit) {
  *(gpio_registerset+(bit/10)) &= ~(7<<((bit%10)*3));  // prepare: set as input
}


int main(int argc, char **argv)
{
	uint32_t last_seqnum=0xffffffff, seqnum; 
	
	
	if (initRadioberry() < 0){
		fprintf(stderr,"Radioberry; could not be initialized. \n");
		exit(-1);
	}
	
	spi_control_rx1_phase();
	
	fprintf(stderr, "speed test no pull up.\n");
	gpioSetMode(6, PI_PUD_OFF);
	int i = 0;
	gettimeofday(&t20, 0);
	for (i=0; i < 192000 ; i++) {
		rx_reader(iqdata);
		
		//seqnum=((iqdata[0]&0xFF)<<16)+((iqdata[1]&0xFF)<<8)+(iqdata[2]&0xFF);
		//fprintf(stderr,"Radioberry: recvd %X %X %X %X %X %X\n",  iqdata[0], iqdata[1], iqdata[2], iqdata[3], iqdata[4], iqdata[5]);
		//fprintf(stderr,"Radioberry: recvd %ld\n",  seqnum);
		
		//if (seqnum != last_seqnum + 1) {
		//  fprintf(stderr,"Radioberry firmware SEQ ERROR: last %ld, recvd %ld\n", (long) last_seqnum, (long) seqnum);
		//  fprintf(stderr,"Radioberry: recvd %X %X %X\n",  iqdata[0], iqdata[1], iqdata[2]);
		//}
		//last_seqnum = seqnum;
	}
	gettimeofday(&t21, 0);
	float elapsd = timedifference_msec(t20, t21);
	fprintf(stderr, "192K samples without pull up  retrieved in %f milliseconds.\n", elapsd);
	fprintf(stderr,"Radioberry: recvd %X %X %X %X %X %X\n",  iqdata[0], iqdata[1], iqdata[2], iqdata[3], iqdata[4], iqdata[5]);
	
//	fprintf(stderr, "speed test pull up.\n");
	int level = 0;
//	gpioSetMode(6, PI_PUD_UP);
//	gettimeofday(&t20, 0);
//	for (i=0; i < 10000000 ; i++) {
//		//rx_reader(iqdata);
//		level = !level;
//		gpioWrite(6, level);
//	}
//	gettimeofday(&t21, 0);
//	elapsd = timedifference_msec(t20, t21);
//	fprintf(stderr, "10000000 read actions with pull up retrieved in %f milliseconds.\n", elapsd);
	
	//closeRadioberry();
	
	
	
	// experiment without library....
	
	fprintf(stderr, "****WITHOUT LIBRARY!\n");
	
	  // Prepare GPIO
	volatile uint32_t *gpio_port = mmap_bcm_register(GPIO_REGISTER_BASE);
	initialize_gpio_for_output(gpio_port, TOGGLE_GPIO);
	volatile uint32_t *set_reg = gpio_port + (GPIO_SET_OFFSET / sizeof(uint32_t));
	volatile uint32_t *clr_reg = gpio_port + (GPIO_CLR_OFFSET / sizeof(uint32_t));
	
	volatile uint32_t *read_reg = gpio_port + (GPIO_LEV0_OFFSET / sizeof(uint32_t));
	eread_reg = read_reg;
	
	initialize_gpio_for_input(gpio_port, 23);
	initialize_gpio_for_input(gpio_port, 20);
	initialize_gpio_for_input(gpio_port, 19);
	initialize_gpio_for_input(gpio_port, 18);
	initialize_gpio_for_input(gpio_port, 16);
	initialize_gpio_for_input(gpio_port, 13);
	initialize_gpio_for_input(gpio_port, 12);
	initialize_gpio_for_input(gpio_port, 5);
	
	initialize_gpio_for_input(gpio_port, 25); //samples avail
	
	uint32_t keep = 0;
	level = 0;
	/*
	gettimeofday(&t20, 0);
	for (i=0; i < 192000 * 6 ; i++) {
		level = !level;
		if (level) 	*set_reg = (1<<TOGGLE_GPIO); else *clr_reg = (1<<TOGGLE_GPIO);
		//usleep(100000);
		uint32_t value = *read_reg;
		//value = *read_reg;
		if (i < 6) {
			iqdata[i]  =  (((value >> 23) & 1) << 7);
			iqdata[i] |=  (((value >> 20) & 1) << 6);
			iqdata[i] |=  (((value >> 19) & 1) << 5);
			iqdata[i] |=  (((value >> 18) & 1) << 4);
			iqdata[i] |=  (((value >> 16) & 1) << 3);
			iqdata[i] |=  (((value >> 13) & 1) << 2);
			iqdata[i] |=  (((value >> 12) & 1) << 1);
			iqdata[i] |=  (((value >> 5) & 1));
		} else {
			keep  =  (((value >> 23) & 1) << 7);
			keep |=  (((value >> 20) & 1) << 6);
			keep |=  (((value >> 19) & 1) << 5);
			keep |=  (((value >> 18) & 1) << 4);
			keep |=  (((value >> 16) & 1) << 3);
			keep |=  (((value >> 13) & 1) << 2);
			keep |=  (((value >> 12) & 1) << 1);
			keep |=  (((value >> 5) & 1));
		}
	}
	fprintf(stderr,"Radioberry: recvd %X %X %X %X %X %X\n",  iqdata[0], iqdata[1], iqdata[2], iqdata[3], iqdata[4], iqdata[5]);
	gettimeofday(&t21, 0);
	elapsd = timedifference_msec(t20, t21);
	fprintf(stderr, "without library 10000000 read actions in %f milliseconds.\n", elapsd);
	*/
	int lcount = 0;
	gettimeofday(&t20, 0);
	//check the availabilty is working this must run the original gateware.
	while(1) {
		//while ((((*eread_reg) >> 25) & 1) == 0) {fprintf(stderr, "+"); usleep(3000);}//wait for enough samples
		while (((*eread_reg >> 25) & 1) == 0) {}//wait for enough samples
		
		
		
		for (i=0; i < 6 ; i++) {
			level = !level;
			if (level) 	*set_reg = (1<<TOGGLE_GPIO); else *clr_reg = (1<<TOGGLE_GPIO);
			
			uint32_t value = *read_reg;
		
			iqdata[i]  =  (((value >> 23) & 1) << 7);
			iqdata[i] |=  (((value >> 20) & 1) << 6);
			iqdata[i] |=  (((value >> 19) & 1) << 5);
			iqdata[i] |=  (((value >> 18) & 1) << 4);
			iqdata[i] |=  (((value >> 16) & 1) << 3);
			iqdata[i] |=  (((value >> 13) & 1) << 2);
			iqdata[i] |=  (((value >> 12) & 1) << 1);
			iqdata[i] |=  (((value >> 5) & 1));
		}
		lcount++;
		//fprintf(stderr,"Radioberry: recvd %X %X %X %X %X %X\n",  iqdata[0], iqdata[1], iqdata[2], iqdata[3], iqdata[4], iqdata[5]);
		
		//while (gpioRead(25) == 0) {fprintf(stderr, "*");}
		if (lcount == 192000) {
			gettimeofday(&t21, 0);
			elapsd = timedifference_msec(t20, t21);
			fprintf(stderr, "192k read actions in %f milliseconds.\n", elapsd);
			gettimeofday(&t20, 0);
			lcount = 0;
		}
		
	}
	
}

int initRadioberry() {

	if (gpioInitialise() < 0) {
		fprintf(stderr,"Radioberry;  gpio could not be initialized. \n");
		exit(-1);
	}
	
	gpioSetMode(25, PI_INPUT); 	//rx samples
	
	// defining the IO ports as input; required for reading bits in one cycle.
	// rx data byte:
	gpioSetMode(23, PI_INPUT); 
	gpioSetMode(20, PI_INPUT);
	gpioSetMode(19, PI_INPUT);
	gpioSetMode(18, PI_INPUT);
	gpioSetMode(16, PI_INPUT);
	gpioSetMode(13, PI_INPUT);
	gpioSetMode(12, PI_INPUT);
	gpioSetMode(5, PI_INPUT); 

	//define control IO ports
	gpioSetMode(6, PI_OUTPUT);  	// pi-clk rx1 slice
	gpioSetMode(17, PI_OUTPUT); 	// pi-clk rx2 slice
	gpioSetMode(21, PI_OUTPUT);  	// ptt
	
	//Initialise
	gpioWrite(21, 0); 				// set ptt off
	gpioWrite(6, 0); 				// init pi-clk1 
	gpioWrite(17, 0); 				// init pi-clk2 
	
	rx1_spi_handler = spiOpen(0, 9000000, 49155);  //channel 0
	if (rx1_spi_handler < 0) {
		fprintf(stderr,"radioberry_protocol: spi bus rx1 could not be initialized. \n");
		exit(-1);
	}	
	printf("Radioberry, Initialisation succesfully executed.\n");
}

void closeRadioberry() {
	if (rx1_spi_handler !=0) spiClose(rx1_spi_handler);

	gpioTerminate();
}


void rx_reader(unsigned char iqdata[]){
	int i = 0;
	int level = 0;
	for (i; i < 6 ; i++) {
		level = !level;
		gpioWrite(6, level);
		
		uint32_t value = gpioRead_Bits_0_31();
		//usleep(400);
		//value = gpioRead_Bits_0_31();
		
		iqdata[i]  =  (((value >> 23) & 1) << 7);
		iqdata[i] |=  (((value >> 20) & 1) << 6);
		iqdata[i] |=  (((value >> 19) & 1) << 5);
		iqdata[i] |=  (((value >> 18) & 1) << 4);
		iqdata[i] |=  (((value >> 16) & 1) << 3);
		iqdata[i] |=  (((value >> 13) & 1) << 2);
		iqdata[i] |=  (((value >> 12) & 1) << 1);
		iqdata[i] |=  (((value >> 5) & 1));
	}
}

void spi_control_rx1_phase() {
	unsigned char iqdata[6];
	
	freq = 1395000;
	rx1_phase = (uint32_t)ceil(((double)freq * 4294967296.0 ) / 76800000);	
	
	iqdata[0] = (0x10 | (sampleSpeed & 0x03));
	iqdata[1] = (((vna << 7) & 0x80) | ((rando << 6) & 0x40) | ((dither <<5) & 0x20) |  (att & 0x1F));
	iqdata[2] = ((rx1_phase >> 24) & 0xFF);
	iqdata[3] = ((rx1_phase >> 16) & 0xFF);
	iqdata[4] = ((rx1_phase >> 8) & 0xFF);
	iqdata[5] = (rx1_phase & 0xFF);
		
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
}

float timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

//end of source.