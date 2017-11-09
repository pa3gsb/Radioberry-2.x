/*
      Radioberry clock
	  
	  This program sets the (fysical pin 36) gpio6 clock pin to 10 Mhz.
	  
	  2017 Johan PA3GSB
*/

#include <stdlib.h>
#include <stdio.h>
#include <pigpio.h>

int main(int argc, char **argv)
{
	if (gpioInitialise() < 0) {
		fprintf(stderr,"gpio could not be initialized. \n");
		exit(-1);
	}
	
	gpioSetMode(6, PI_ALT0);
	
	int result = gpioHardwareClock(6, 10000000);
	
	printf("result = %d\n", result);

		
	gpioTerminate();
}

