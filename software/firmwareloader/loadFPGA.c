#include <stdio.h>
#include <wiringPi.h>

#include "loadFPGA.h"


#define S_CUR				1 /* SEEK_CUR */
#define S_END				2 /* SEEK_END */
#define S_SET				0 /* SEEK_SET */


#define iPinCONF_DONE 		3	
#define iPinNSTATUS 		25	

#define oPinNCONFIG 		2	
#define oPinDATA 			23	
#define oPinDCLK 			5	


int main (void)
{
	  printf ("Raspberry Pi - RadioBerry Firmware Loader\n") ;
	  printf ("==============================\n") ;
	  printf ("\n") ;

	  int file_id;
	  
	  wiringPiSetup () ;
	  
	  pinMode (iPinCONF_DONE, INPUT);
	  pinMode (iPinNSTATUS, INPUT);
	  
	  pinMode (oPinNCONFIG, OUTPUT);
	  pinMode (oPinDATA, OUTPUT);
	  pinMode (oPinDCLK, OUTPUT);

	/* Open programming file as READ and in BINARY */
	file_id = fopen_rbf( "radioberry.rbf", "rb" );

	if ( file_id )
		fprintf( stdout, "Info: Programming file: \"%s\" opened...\n", "radioberry.rbf" );
	else
	{
		fprintf( stderr, "Error: Could not open programming file: \"%s\"\n", "radioberry.rbf" );
		return -1;
	}


	/* Processing and Programming */
	processFileInput( file_id );


	if ( file_id )
		fclose_rbf(file_id);
  
  
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
	if (digitalRead(iPinNSTATUS) == 0) {
		fprintf( stderr, "Error: programming failed; nstatus is low\n" );
		return;
	} else if(digitalRead(iPinCONF_DONE) == 0) {
			fprintf( stderr, "Error: programming failed; conf done is low\n" );
			return;
	}
		
	/* Initialize device */
	digitalWrite (oPinDCLK, 1);
	digitalWrite (oPinDCLK, 0);
	digitalWrite (oPinDCLK, 1);
	digitalWrite (oPinDCLK, 0);
	
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
		
		digitalWrite (oPinDATA, bit);
		digitalWrite (oPinDCLK, 1);
		digitalWrite (oPinDCLK, 0);
	}
}


int prepareLoading()  
{
	fprintf( stdout, "Info: prepareLoading\n" );
	
	//fprintf( stdout, "Info: iPinNstatus is : %d\n", digitalRead(iPinNSTATUS) );
	
	digitalWrite (oPinNCONFIG, 0) ; 
	digitalWrite (oPinDATA, 0) ; 	
	digitalWrite (oPinDCLK, 0) ; 	
	delay(1);
	digitalWrite (oPinNCONFIG, 1) ; 

	int count = 0;
	while (digitalRead(iPinNSTATUS) == 0) {
		count++;
		delay(10);
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