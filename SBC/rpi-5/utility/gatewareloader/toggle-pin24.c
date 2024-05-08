#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <gpiod.h>
// this utility relies on libgpiod.so.2 as delivered by installing libgpiod2 from Raspi Debian 12 (bookworm)
// use LDFLAG -l:libgpiod.so.2

#define iPinCONF_DONE 		22
#define iPinNSTATUS 		26

#define oPinNCONFIG 		27
#define oPinDATA 		13
#define oPinDCLK 		24

#ifndef CONSUMER
#define CONSUMER	"GPIODTest"
#endif

struct gpiod_line *pin22done, *pin26status;		// inputs
struct gpiod_line *pin27config, *pin13data, *pin24dclk; // outputs

void togglethepin();

// by using libgpiod, the user account needs only be a member of the gpio group.
// the default rpi user 'pi' is a member of group gpio
// therefore: NO 'sudo' needed!

int main (int argc, char **argv)
{
	printf ("\n\n");
	printf ("===============================================\n") ;
	printf ("===  Raspberry Pi 5 - GPIO 24 Pin Toggler   ===\n") ;
	printf ("===============================================\n") ;
	printf ("\n\n");

	char *chipname = "gpiochip4";
	struct gpiod_chip *chip;

	if ((chip = gpiod_chip_open_by_name( chipname )) == NULL) {
		fprintf(stderr, "Unable to open %s\n", chipname);
		goto end;
	}

	if ((pin22done = gpiod_chip_get_line( chip, iPinCONF_DONE )) == NULL) {
		fprintf(stderr, "Unable to get pin%d\n", iPinCONF_DONE);
		goto close_chip;
	}
	if ((pin26status = gpiod_chip_get_line( chip, iPinNSTATUS )) == NULL) {
		fprintf(stderr, "Unable to get pin%d\n", iPinNSTATUS);
		goto close_chip;
	}
	if ((pin27config = gpiod_chip_get_line( chip, oPinNCONFIG )) == NULL) {
		fprintf(stderr, "Unable to get pin%d\n", oPinNCONFIG);
		goto close_chip;
	}
	if ((pin13data = gpiod_chip_get_line( chip, oPinDATA )) == NULL) {
		fprintf(stderr, "Unable to get pin%d\n", oPinDATA);
		goto close_chip;
	}
	if ((pin24dclk = gpiod_chip_get_line( chip, oPinDCLK )) == NULL) {
		fprintf(stderr, "Unable to get pin%d\n", oPinDCLK);
		goto close_chip;
	}

	if (gpiod_line_request_input( pin22done, CONSUMER ) < 0) {
		gpiod_line_release( pin22done );
		fprintf(stderr, "Request pin%d as input failed\n", iPinCONF_DONE);
		goto close_chip;
	}
	if (gpiod_line_request_input( pin26status, CONSUMER ) < 0) {
		gpiod_line_release( pin26status );
		fprintf(stderr, "Request pin%d as input failed\n", iPinNSTATUS);
		goto close_chip;
	}

	if (gpiod_line_request_output( pin27config, CONSUMER, 0 ) < 0) { // initial pin value = 0
		gpiod_line_release( pin27config );
		fprintf(stderr, "Request pin%d as output failed\n", oPinNCONFIG);
		goto close_chip;
	}
	if (gpiod_line_request_output( pin13data, CONSUMER, 0 ) < 0) {
		gpiod_line_release( pin13data );
		fprintf(stderr, "Request pin%d as output failed\n", oPinDATA);
		goto close_chip;
	}
	if (gpiod_line_request_output( pin24dclk, CONSUMER, 0 ) < 0) {
		gpiod_line_release( pin24dclk );
		fprintf(stderr, "Request pin%d as output failed\n", oPinDCLK);
		goto close_chip;
	}

	fprintf(stderr, "Init %s success....\n", chipname);

	// Processing and Programming
	togglethepin();

close_chip:
	gpiod_chip_close( chip );
end:
	return 0;
}

void togglethepin ()
{
	fprintf(stderr, "GPIO 24 will be toggled as fast as we can\n");
	fprintf(stderr, "starting in 5 us....\n");

	struct timespec Dly = { (time_t) 0L, (time_t) 5000L }; // 5 us

	gpiod_line_set_value( pin24dclk, 0 );
	nanosleep( &Dly, NULL );

	int count = 0;
	Dly.tv_nsec = (time_t) 1L; // 1 ns

	for ( ; count < 50000; count++ ) {
		nanosleep( &Dly, NULL );
		gpiod_line_set_value( pin24dclk, 1 );
		nanosleep( &Dly, NULL );
		gpiod_line_set_value( pin24dclk, 0 );
//		if ((++count % 5000) == 0) fprintf(stderr, "."); // goes slower with output to stderr
	}
	fprintf(stderr, "\n");

	Dly.tv_nsec = (time_t) 5000L; // 5 us
	nanosleep( &Dly, NULL );

	fprintf(stderr, "Done toggling GPIO 24....\n");
}
