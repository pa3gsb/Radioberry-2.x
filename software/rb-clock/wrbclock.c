
#include <stdio.h>
#include <wiringPi.h>

#define GPCLK0	4		// pin 7
#define GPCLK1	5		// pin 29 
#define	GPCLK2	6		// pin 31 

int main() {
	wiringPiSetupGpio();
	pinMode(GPCLK2, GPIO_CLOCK);
	gpioClockSet(GPCLK2, 10000000);
	return 0;
}