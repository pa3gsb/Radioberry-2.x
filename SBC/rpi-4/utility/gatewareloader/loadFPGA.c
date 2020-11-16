#include <stdlib.h>
#include <stdio.h>
#include <pigpio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "loadFPGA.h"


#define S_CUR				1 /* SEEK_CUR */
#define S_END				2 /* SEEK_END */
#define S_SET				0 /* SEEK_SET */


#define iPinCONF_DONE 		22	
#define iPinNSTATUS 		26	

#define oPinNCONFIG 		27	
#define oPinDATA 			13	
#define oPinDCLK 			24

int rpi4;


int main (int argc, char **argv)
{
	printf ("\n\n");
	printf ("=======================================================\n") ;
	printf ("===    Raspberry Pi - RadioBerry Firmware Loader    ===\n") ;
	printf ("=======================================================\n") ;
	printf ("\n\n");

	if (argc != 2)
	{
      printf("Can not execute, valid command line arguments are 'CL016' or 'CL025'\n\n");
      return -1;   
	}
	  
	  
	char filename[30];

	if (strcmp(argv[1], "CL016")==0) {
		strcpy(filename, "radioberry-10CL016.rbf");
	} else if (strcmp(argv[1], "CL025")==0) {
		strcpy(filename, "radioberry.rbf");
	} else {
      printf("Can not execute!. \nCommand line argument expected by program must be 'CL016' or 'CL025'\n\n");
	  return -1;
	}
	
	printf("%s  will be loaded.\n\n\n", filename);

	int file_id;
	
	unsigned int rev = gpioHardwareRevision();  
	//A03111 hex
	int rpi4 = (rev >= 10498321) ? 1 : 0;
	if (rpi4) fprintf(stderr, "\nLoading radioberry firmware using a RPI-4\n\n");

	gpioVersion() ;
	printf ("pigpio version: %d\n\n", gpioVersion()) ;
	
	if (gpioInitialise() < 0) {
		fprintf(stderr,"loading radioberry firmware : gpio could not be initialized. \n");
		exit(-1);
	}
	
	gpioSetMode(iPinCONF_DONE, PI_INPUT); 	
	gpioSetMode(iPinNSTATUS, PI_INPUT); 
	
	gpioSetMode(oPinNCONFIG, PI_OUTPUT); 
	gpioSetMode(oPinDATA, PI_OUTPUT); 
	gpioSetMode(oPinDCLK, PI_OUTPUT); 


	/* Open programming file as READ and in BINARY */
	file_id = fopen_rbf( filename, "rb" );

	if ( file_id )
		fprintf( stdout, "Info: Programming file: \"%s\" opened...\n", filename );
	else
	{
		fprintf( stderr, "Error: Could not open programming file: \"%s\"\n", filename );
		return -1;
	}


	/* Processing and Programming */
	processFileInput( file_id );


	if ( file_id )
		fclose_rbf(file_id);
  
	gpioTerminate();
  
	return 0;
}

void processFileInput( int finputid )
{
	int			seek_position = 0;			/* file pointer position */
	long int	file_size = 0;				/* programming file size */
	int			one_byte = 0;				/* the byte to program */
	long int	i = 0;						/* standard counter variable */
	
	/* Get file size */
	seek_position = fseek_rbf( finputid, 0, S_END );

	if ( seek_position )
	{
		fprintf( stderr, "Error: End of file could not be located!\n" );
		return;
	}

	file_size	= ftell_rbf( finputid );
	fprintf( stdout, "Info: Programming file size: %ld\n", file_size );
	
	
	fseek_rbf( finputid, 0, S_SET );
	
	/* Start configuration */
	if (prepareLoading() == -1) return;
	
	/* Loop through every single byte */
	for ( i = 0; i < file_size; i++ )
	{
		one_byte = fgetc_rbf( finputid );
		programByte( one_byte );
	}
	
	/* Check if loading succeeded*/
	if (gpioRead(iPinNSTATUS) == 0) {
		fprintf( stderr, "Error: programming failed; nstatus is low\n" );
		return;
	} else if(gpioRead(iPinCONF_DONE) == 0) {
			fprintf( stderr, "Error: programming failed; conf done is low\n" );
			return;
	}
		
	/* Initialize device */
	gpioWrite(oPinDCLK, 1);
	gpioWrite(oPinDCLK, 0);
	gpioWrite(oPinDCLK, 1);
	gpioWrite(oPinDCLK, 0);
	
	fprintf( stdout, "Info: programming succeeded\n" );
}

void programByte( int one_byte )
{
	int	bit = 0;
	int i = 0;

	/* write from LSb to MSb */
	for ( i = 0; i < 8; i++ )
	{
		bit = one_byte >> i;
		bit = bit & 0x1;
		
		gpioWrite(oPinDATA, bit);
		gpioWrite(oPinDCLK, 1);
		gpioWrite(oPinDCLK, 0);
	}
}


int prepareLoading()  
{
	fprintf( stdout, "Info: prepareLoading\n" );
	
	gpioWrite (oPinNCONFIG, 0) ; 
	gpioWrite (oPinDATA, 0) ; 	
	gpioWrite (oPinDCLK, 0) ; 	
	sleep(1);
	gpioWrite (oPinNCONFIG, 1) ; 

	int count = 0;
	while (gpioRead(iPinNSTATUS) == 0) {
		count++;
		sleep(10);
		if (count >= 255) {
			fprintf( stderr, "Error: prepareLoading failed\n" );
			return -1;
		}
	}

	return 0;
}

int fopen_rbf( char argv[], char* mode)
{
	FILE* file_id;

	file_id = fopen( argv, mode );

	return (int) file_id;
}

int	fclose_rbf( int file_id)
{
	fclose( (FILE*) file_id);

	return 0;
}

int fseek_rbf( int finputid, int start, int end )
{
	int seek_position;

	seek_position = fseek( (FILE*) finputid, start, end );

	return seek_position;
}

int ftell_rbf( int finputid )
{
	int file_size;

	file_size = ftell( (FILE*) finputid );

	return file_size;
}

int fgetc_rbf( int finputid )
{
	int one_byte;

	one_byte = fgetc( (FILE*) finputid );

	return one_byte;
}