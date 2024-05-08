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
// this utility relies on libgpiod.so.3 as delivered by cloning libgpiod.git repo
// git clone https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git
// installation ends up in /usr/local

// work is not completed. this only begins the changes for the libgpiod v3 API

#define iPinCONF_DONE 		22
#define iPinNSTATUS 		26

#define oPinNCONFIG 		27
#define oPinDATA 		13
#define oPinDCLK 		24

#ifndef CONSUMER
#define CONSUMER	"Radioberry"
#endif

struct gpiod_line *pin22done, *pin26status;		// inputs
struct gpiod_line *pin27config, *pin13data, *pin24dclk; // outputs

void processRBF();
void programByte( unsigned char );
int prepareLoading();

// by using libgpiod, the user account needs only be a member of the gpio group.
// the default rpi user 'pi' is a member of group gpio
// therefore: NO 'sudo' needed!

int main (int argc, char **argv)
{
	printf ("\n\n");
	printf ("=======================================================\n") ;
	printf ("===  Raspberry Pi 5 - Radioberry Gateware Loader    ===\n") ;
	printf ("=======================================================\n") ;
	printf ("\n\n");

	char *chippath = "/dev/gpiochip4"; // /dev/gpiochip0 for all RPiX where X < 5
	struct gpiod_chip *chip;

	if ((chip = gpiod_chip_open( chippath )) == NULL) {
		fprintf(stderr, "Unable to open %s\n", chippath);
		goto end;
	}
	if ((pin22done = gpiod_chip_get_line( chip, iPinCONF_DONE )) == NULL) {
		fprintf(stderr, "Unable to get pin%d\n", iPinCONF_DONE);
		goto close_chip;
	}
#ifdef NEWCODE
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
	processRBF();
#endif
close_chip:
	gpiod_chip_close( chip );
end:
	return 0;
}

void processRBF ()
{
	fprintf(stderr, "radioberry.rbf will be loaded\n");

	FILE *finp;
	// Open programming file as READ and in BINARY
	if ((finp = fopen( "radioberry.rbf", "rb" )) == NULL) {
		fprintf(stderr, "File not found\n");
		return;
	}
	else {
		int fd = fileno( finp );
		struct stat sb;
		fstat( fd, &sb );
		fprintf(stderr, "\nGateware size  %jd bytes\n\n", (intmax_t) sb.st_size );
	}

	fprintf(stderr, "prepare FPGA for gateware....\n");

	// Start configuration
	if (prepareLoading() < 0) return;

	fprintf(stderr, "gateware loading now....\n");
	usleep(10);

	unsigned char byte = 0;
	int count = 0;
	while (fread(&byte, sizeof(unsigned char), 1, finp) == 1) {
		programByte( byte );
		if ((++count % 5000) == 0) fprintf(stderr, ".");
	}
	fprintf(stderr, "\n");
	fclose( finp ); // if we are here, finp != NULL

	usleep(10);
#ifdef NEWCODE
	/* Check if loading succeeded*/
	if (gpiod_line_get_value( pin26status ) == 0) {
		fprintf(stderr, "Error: programming failed; pin26status != 1\n");
		return;
	} else if (gpiod_line_get_value( pin22done ) == 0) {
		fprintf(stderr, "Error: programming failed; pin22done != 1\n");
		return;
	}

	// toggle DCLK three more times (should always check return value?)
	gpiod_line_set_value( pin24dclk, 1 );
	usleep(1);
	gpiod_line_set_value( pin24dclk, 0 );
	usleep(1);
	gpiod_line_set_value( pin24dclk, 1 );
	usleep(1);
	gpiod_line_set_value( pin24dclk, 0 );
	usleep(1);
	gpiod_line_set_value( pin24dclk, 1 );
	usleep(1);
	gpiod_line_set_value( pin24dclk, 0 );

	fprintf(stderr, "gateware load success....\n");
#endif
}

//struct gpiod_line *pin22done, *pin26status;		// inputs
//struct gpiod_line *pin27config, *pin13data, *pin24dclk; // outputs
void programByte( unsigned char one_byte )
{
	char bit = 0;
	int i = 0;

	// write bits from LSb to MSb
	for ( i = 0; i < 8; i++ )
	{
		bit = one_byte >> i;
		bit = bit & 0x1;
#ifdef NEWCODE
		gpiod_line_set_value( pin13data, bit );
//		usleep(1);

		gpiod_line_set_value( pin24dclk, 1 );
		usleep(1);
		gpiod_line_set_value( pin24dclk, 0 );
//		usleep(1);
#endif
	}
}

int prepareLoading ()
{
	fprintf(stdout, "Info: prepareLoading\n");

	// pins 27, 13, and 24 initial values all = 0
#ifdef DEBUG
	// these pins were already initialized when output mode was set
	if (gpiod_line_set_value( pin27config, 0 ) < 0) {
		fprintf(stderr, "Did not set pin%d to 0\n", oPinNCONFIG);
		return -1;
	}
	if (gpiod_line_set_value( pin13data, 0 ) < 0) {
		fprintf(stderr, "Did not set pin%d to 0\n", oPinDATA);
		return -1;
	}
	if (gpiod_line_set_value( pin24dclk, 0 ) < 0) {
		fprintf(stderr, "Did not set pin%d to 0\n", oPinDCLK);
		return -1;
	}
	usleep(1);
#endif
#ifdef NEWCODE
	if (gpiod_line_set_value( pin27config, 1 ) < 0) {
		fprintf(stderr, "Did not set pin%d to 1\n", oPinNCONFIG);
		return -1;
	}
	usleep(1);

	int count = 0;
	while (gpiod_line_get_value( pin26status ) == 0) {
		count++;
		usleep(5);
		if (count >= 255) {
			fprintf(stderr, "Error: prepareLoading failed\n");
			return -1;
		}
	}

	fprintf(stdout, "Info: prepareLoading success\n");
#endif
	return 0;
}
