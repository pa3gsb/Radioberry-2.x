#include <stdlib.h>
#include <stdio.h>
#include <pigpio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>


#include "loadFPGA.h"


#define iPinCONF_DONE 		22	
#define iPinNSTATUS 		26	

#define oPinNCONFIG 		27	
#define oPinDATA 			13	
#define oPinDCLK 			24


volatile  uint32_t *GPIOBase = NULL ;
volatile  uint32_t *RIOBase = NULL ;
volatile  uint32_t *PADBase = NULL ;
volatile   uint32_t *Pad = NULL ; 


#define GPIO_FUNC_RIO 5
#define PAD_FUNC_IN 0xc8
#define PAD_FUNC_OUT 0x10

typedef struct{
    uint32_t status;
    uint32_t ctrl; 
}GPIOregs;

#define GPIO ((GPIOregs*)GPIOBase)
typedef struct
{
    uint32_t out;
    uint32_t oe;
    uint32_t in;
    uint32_t InSync;
} rioregs;

#define rio ((rioregs *)RIOBase)
#define rioXOR ((rioregs *)(RIOBase + 0x1000 / 4))
#define rioSET ((rioregs *)(RIOBase + 0x2000 / 4))
#define rioCLR ((rioregs *)(RIOBase + 0x3000 / 4))


static void initialize_gpio_for_output(uint32_t pin) {
    GPIO[pin].ctrl=GPIO_FUNC_RIO;
    Pad[pin] = PAD_FUNC_OUT;
    rioSET->oe = 0x01<<pin; // output driver
}

static void initialize_gpio_for_input(uint32_t pin) {
	GPIO[pin].ctrl=GPIO_FUNC_RIO;
    Pad[pin] = PAD_FUNC_IN;
    //rioSET->in = 0x01<<pin;
	rioCLR->oe = 0x01<<pin; //high impedance
}

static void set_pin(uint32_t pin) {
	rioSET->oe = 0x01<<pin; // output driver
	rioSET->out = 0x01<<pin;
}

static void clr_pin(uint32_t pin) {
	rioSET->oe = 0x01<<pin; // output driver
	rioCLR->out = 0x01<<pin;
}

static uint32_t read_pin(uint32_t pin) {
	return (rio->in>>pin & 0x01);
}

static uint32_t get_pinhigh(uint32_t pin) {
	return (GPIO[pin].status & 0x80000);
}

static uint32_t get_pinlow(uint32_t pin) {
	return (GPIO[pin].status & 0x40000);
}


int main (int argc, char **argv)
{
	printf ("\n\n");
	printf ("=======================================================\n") ;
	printf ("===  Raspberry Pi 5 - RadioBerry Gateware Loader    ===\n") ;
	printf ("=======================================================\n") ;
	printf ("\n\n");
 	
    int memfd = open("/dev/mem", O_RDWR | O_SYNC);
    uint32_t *map = (uint32_t *)mmap(
        NULL,
        64 * 1024 * 1024,
        (PROT_READ | PROT_WRITE),
        MAP_SHARED,
        memfd,
        0x1f00000000
    );
    if (map == MAP_FAILED)
    {
        printf("mmap failed: %s\n", strerror(errno));
        return (-1);
    };
    close(memfd);
    uint32_t *PERIBase = map;
    GPIOBase = PERIBase + 0xD0000 / 4;
	RIOBase = PERIBase + 0xe0000 / 4;
    PADBase = PERIBase + 0xf0000 / 4;
    Pad = PADBase + 1;   
	
	initialize_gpio_for_input(iPinCONF_DONE);
	initialize_gpio_for_input(iPinNSTATUS);
	
	initialize_gpio_for_output(oPinNCONFIG);
	initialize_gpio_for_output(oPinDATA);
	initialize_gpio_for_output(oPinDCLK);
		
	fprintf(stderr, "Init done.\n");
	
	/* Processing and Programming */
	processFileInput(  );
   
	return 0;
}

void processFileInput( )
{
	int			seek_position = 0;			/* file pointer position */
	long int	file_size = 0;				/* programming file size */
	int			one_byte = 0;				/* the byte to program */
	long int	i = 0;						/* standard counter variable */
	
	char filename[30];
	strcpy(filename, "radioberry.rbf");
	printf("%s  will be loaded.\n", filename);

	FILE *finputid;
	
	/* Open programming file as READ and in BINARY */
	finputid = fopen( "radioberry.rbf", "rb" );
	if (finputid == NULL) {
        perror("Error opening file");
        return;
    }

	/* Start configuration */
	if (prepareLoading() == -1) return;
	
	fprintf(stderr, "now gateware loading....\n");
	
	sleep(1);
		
	unsigned char byte; 
	while (fread(&byte, sizeof(unsigned char), 1, finputid) == 1) {
		programByte( byte );
	}	
	
	//fprintf(stderr, "nstatus level = %d \n", read_pin(iPinNSTATUS));
	//fprintf(stderr, "conf level = %d \n", read_pin(iPinCONF_DONE));
	
	sleep(1);
	
	/* Check if loading succeeded*/
	if (read_pin(iPinNSTATUS) == 0) {
		fprintf( stderr, "Error: programming failed; nstatus is low\n" );
		return;
	} else if (read_pin(iPinCONF_DONE) == 0) {
		fprintf( stderr, "Error: programming failed; conf done is low\n" );
		return;
	}
		
	set_pin(oPinDCLK);
	usleep(1);
	clr_pin(oPinDCLK);
	usleep(1);
	set_pin(oPinDCLK);
	usleep(1);
	clr_pin(oPinDCLK);
	usleep(1);
	set_pin(oPinDCLK);
	usleep(1);
	clr_pin(oPinDCLK);
	
	fprintf( stdout, "gateware loaded succeeded\n" );
	
	if ( finputid )	fclose(finputid);
}


void programByte( unsigned char one_byte )
{
	char	bit = 0;
	int i = 0;

	/* write from LSb to MSb */
	for ( i = 0; i < 8; i++ )
	{
		bit = one_byte >> i;
		bit = bit & 0x1;
					
		if (bit) set_pin(oPinDATA); else clr_pin(oPinDATA); 
		// fprintf(stderr, "");
		usleep(1);
		set_pin(oPinDCLK);
		usleep(1);
		clr_pin(oPinDCLK);
		usleep(1);
	}
}


int prepareLoading()  
{
	fprintf( stdout, "Info: prepareLoading\n" );
	
	//fprintf(stderr, "nstatus level = %d \n", read_pin(iPinNSTATUS));
	//fprintf(stderr, "conf level = %d \n", read_pin(iPinCONF_DONE));
	
	clr_pin(oPinNCONFIG);
	clr_pin(oPinDATA);
	clr_pin(oPinDCLK); 	
	usleep(1);
	set_pin(oPinNCONFIG);
	usleep(1);

	int count = 0;
	while (read_pin(iPinNSTATUS) == 0) {
		count++;
		usleep(10);
		if (count >= 255) {
			fprintf( stderr, "Error: prepareLoading failed\n" );
			return -1;
		}
	}
	
	//fprintf(stderr, "nstatus level = %d \n", read_pin(iPinNSTATUS));
	//fprintf(stderr, "conf level = %d \n", read_pin(iPinCONF_DONE));
	fprintf( stdout, "Info: prepareLoading done.\n" );

	return 0;
}
