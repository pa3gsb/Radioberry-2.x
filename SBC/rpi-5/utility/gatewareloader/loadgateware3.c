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

struct gpiod_line_request *pin22done, *pin26status;		// inputs
struct gpiod_line_request *pin27config, *pin13data, *pin24dclk; // outputs

void processRBF();
void programByte( unsigned char );
int prepareLoading();

// by using libgpiod, the user account needs only be a member of the gpio group.
// the default rpi user 'pi' is a member of group gpio
// therefore: NO 'sudo' needed!

// useful utility to run in a second shell (X is chip nr. 0 or 4):
// gpionotify -c X --localtime 13 27 26 22 24

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

	struct gpiod_request_config *grconf = gpiod_request_config_new();
	const char *consumer = CONSUMER;

	gpiod_request_config_set_consumer( grconf, consumer );

	struct gpiod_line_config *glconf = gpiod_line_config_new();
	struct gpiod_line_settings *glset = gpiod_line_settings_new();

	gpiod_line_settings_set_direction( glset, GPIOD_LINE_DIRECTION_INPUT );
	unsigned int *gloffset = (void *)&((unsigned int [1]) { iPinCONF_DONE });
	gpiod_line_config_add_line_settings( glconf, gloffset, 1, glset );

	if ((pin22done = gpiod_chip_request_lines( chip, grconf, glconf )) == NULL) {
		fprintf(stderr, "Unable to get pin%d\n", iPinCONF_DONE);
		goto close_chip;
	}

	gpiod_line_config_reset( glconf );
	gloffset = (void *)&((unsigned int [1]) { iPinNSTATUS });
	gpiod_line_config_add_line_settings( glconf, gloffset, 1, glset );

	if ((pin26status = gpiod_chip_request_lines( chip, grconf, glconf )) == NULL) {
		fprintf(stderr, "Unable to get pin%d\n", iPinNSTATUS);
		goto close_chip;
	}

	gpiod_line_config_reset( glconf );
	gpiod_line_settings_set_direction( glset, GPIOD_LINE_DIRECTION_OUTPUT );
	gloffset = (void *)&((unsigned int [1]) { oPinNCONFIG });
	gpiod_line_config_add_line_settings( glconf, gloffset, 1, glset );
	enum gpiod_line_value *glval = (void *)&((enum gpiod_line_value [1]) { GPIOD_LINE_VALUE_INACTIVE });
	gpiod_line_config_set_output_values( glconf, glval, 1 ); // initial pin value = 0

	if ((pin27config = gpiod_chip_request_lines( chip, grconf, glconf )) == NULL) {
		fprintf(stderr, "Unable to get pin%d\n", oPinNCONFIG);
		goto close_chip;
	}

	gpiod_line_config_reset( glconf );
	gloffset = (void *)&((unsigned int [1]) { oPinDATA });
	gpiod_line_config_add_line_settings( glconf, gloffset, 1, glset );
	gpiod_line_config_set_output_values( glconf, glval, 1 ); // initial pin value = 0

	if ((pin13data = gpiod_chip_request_lines( chip, grconf, glconf )) == NULL) {
		fprintf(stderr, "Unable to get pin%d\n", oPinDATA);
		goto close_chip;
	}

	gpiod_line_config_reset( glconf );
	gloffset = (void *)&((unsigned int [1]) { oPinDCLK });
	gpiod_line_config_add_line_settings( glconf, gloffset, 1, glset );
	gpiod_line_config_set_output_values( glconf, glval, 1 ); // initial pin value = 0

	if ((pin24dclk = gpiod_chip_request_lines( chip, grconf, glconf )) == NULL) {
		fprintf(stderr, "Unable to get pin%d\n", oPinDCLK);
		goto close_chip;
	}

	fprintf(stderr, "Init %s success....\n", chippath);

	// Processing and Programming
	processRBF();

	gpiod_line_request_release( pin24dclk ); // upon release: alloc vanishes from gpioinfo -c X
	gpiod_line_request_release( pin13data );
	gpiod_line_request_release( pin27config );
	gpiod_line_request_release( pin26status );
	gpiod_line_request_release( pin22done );

close_chip:
	gpiod_line_settings_free( glset );
	gpiod_line_config_free( glconf );
	gpiod_request_config_free( grconf );
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

	// Check if loading succeeded
	// enum gpiod_line_value -- GPIOD_LINE_VALUE_ACTIVE and GPIOD_LINE_VALUE_INACTIVE
	if (gpiod_line_request_get_value( pin26status, iPinNSTATUS ) == GPIOD_LINE_VALUE_INACTIVE) {
		fprintf(stderr, "Error: programming failed; pin26status != Active\n");
		return;
	} else if (gpiod_line_request_get_value( pin22done, iPinCONF_DONE ) == GPIOD_LINE_VALUE_INACTIVE) {
		fprintf(stderr, "Error: programming failed; pin22done != Active\n");
		return;
	}

	// toggle DCLK three more times (should always check return value?)
	gpiod_line_request_set_value( pin24dclk, oPinDCLK, GPIOD_LINE_VALUE_ACTIVE );
	usleep(1);
	gpiod_line_request_set_value( pin24dclk, oPinDCLK, GPIOD_LINE_VALUE_INACTIVE );
	usleep(1);
	gpiod_line_request_set_value( pin24dclk, oPinDCLK, GPIOD_LINE_VALUE_ACTIVE );
	usleep(1);
	gpiod_line_request_set_value( pin24dclk, oPinDCLK, GPIOD_LINE_VALUE_INACTIVE );
	usleep(1);
	gpiod_line_request_set_value( pin24dclk, oPinDCLK, GPIOD_LINE_VALUE_ACTIVE );
	usleep(1);
	gpiod_line_request_set_value( pin24dclk, oPinDCLK, GPIOD_LINE_VALUE_INACTIVE );

	fprintf(stderr, "gateware load success....\n");
}

//struct gpiod_line_request *pin22done, *pin26status;		// inputs
//struct gpiod_line_request *pin27config, *pin13data, *pin24dclk; // outputs

void programByte( unsigned char one_byte )
{
	char bit = 0;
	int i = 0;

	// write bits from LSb to MSb
	for ( i = 0; i < 8; i++ )
	{
		bit = one_byte >> i;
		bit = bit & 0x1;

		gpiod_line_request_set_value( pin13data, oPinDATA,
			(bit ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE) );
//		usleep(1);

		gpiod_line_request_set_value( pin24dclk, oPinDCLK, GPIOD_LINE_VALUE_ACTIVE );
		usleep(1);
		gpiod_line_request_set_value( pin24dclk, oPinDCLK, GPIOD_LINE_VALUE_INACTIVE );
//		usleep(1);
	}
}

int prepareLoading ()
{
	fprintf(stdout, "Info: prepareLoading\n");

	// pins 27, 13, and 24 initial values all = 0

	if (gpiod_line_request_set_value( pin27config, oPinNCONFIG, GPIOD_LINE_VALUE_ACTIVE ) < 0) {
		fprintf(stderr, "Did not set pin%d to Active\n", oPinNCONFIG);
		return -1;
	}
	usleep(1);

	int count = 0;
	while (gpiod_line_request_get_value( pin26status, iPinNSTATUS ) == GPIOD_LINE_VALUE_INACTIVE) {
		count++;
		usleep(5);
		if (count >= 255) {
			fprintf(stderr, "Error: prepareLoading failed\n");
			return -1;
		}
	}

	fprintf(stdout, "Info: prepareLoading success\n");

	return 0;
}
