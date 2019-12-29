/*
      	Radioberry supporting clock
	  
	This program sets the (fysical pin 7) GPCLK0 pin to 10 Mhz.

	This supporting clock is used for:
		-) reset handler
		-) iambic (cw keying) 
		-) heartbeat
		-) ad9866 spi controlling

	It is an important clock but the timing is not used for
	the ADC and DAC conversions.

	This program must be called (clock must be set first) before loading the firmware.
	  
	2018 Johan PA3GSB
*/

#include <stdlib.h>
#include <stdio.h>
#include <pigpio.h>

#define GPCLK0	4		// pin 7
#define GPCLK1	5		// pin 29 DO NOT USE => ADVICED NOT TO USE!
#define	GPCLK2	6		// pin 31 DO NOT USE => RESULTS IN ETHERNET CONNECTION PROBLEMS.

int main(int argc, char **argv)
{
	if (gpioInitialise() < 0) {
		fprintf(stderr,"gpio could not be initialized. \n");
		exit(-1);
	}
	
	gpioSetMode(GPCLK0, PI_ALT0);
	
	int result = gpioHardwareClock(GPCLK0, 10000000);
	
	if (result == 0) 
		printf("Radioberry supporting clock succesfully set to 10 MHz\n");
	else
		printf("Setting radioberry supporting clock FAILED!!!!!!! \n");	
	
	gpioTerminate();
}

