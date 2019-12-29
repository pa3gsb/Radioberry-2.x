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
#include <wiringPi.h>

#define GPCLK0	7		// pin 7
#define GPCLK1	21		// pin 29 DO NOT USE => ADVICED NOT TO USE!
#define	GPCLK2	22		// pin 31 DO NOT USE => RESULTS IN ETHERNET CONNECTION PROBLEMS.

int main(int argc, char **argv)
{
	wiringPiSetup();		
	
	pinMode(GPCLK0, GPIO_CLOCK);
	
	gpioClockSet(GPCLK0, 10000000);
	
	printf("Radioberry supporting clock succesfully set to 10 MHz\n");
	
	return 0;
}

