/*
    WSPRBerry
	  
	Turning a raspberry pi together with the radioberry into a multichannel wspr receiver.
	
	2017 Johan PA3GSB
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pigpio.h>
#include <unistd.h>


#define NRX 3			// number of wspr channels.
#define GAIN 2500		// Digital gain for input signals.

// UTC timing.... important for wspr mode! Starting at even minutes!
time_t rawtime;
struct tm *info;

//wsprnet.org
//Band 	Dial freq (MHz) 	Tx freq (MHz)				Radioberry frequency (+ 1500)
//160m 	1.836600 			1.838000 - 1.838200			 	1.838100
//80m 	3.592600 			3.594000 - 3.594200			 	3.594100
//60m 	5.287200 			5.288600 - 5.288800				5.288700
//40m 	7.038600 			7.040000 - 7.040200			 	7.040100
//30m 	10.138700 			10.140100 - 10.140300			10.140200    
//20m 	14.095600 			14.097000 - 14.097200			14.097100
//17m 	18.104600 			18.106000 - 18.106200			18.106100
//15m 	21.094600 			21.096000 - 21.096200			21.096100
//12m 	24.924600			24.926000 - 24.926200			24.926100
//10m 	28.124600			28.126000 - 28.126200			28.126100

int freqArray[4] = {7040100 , 3594100, 10140200 , 14097100 };

void record_WSPR_Channels(void);
static int spi_handler;
unsigned char iqdata[6];

int main(int argc, char **argv)
{
	printf("\nStart listening to WSPR channel(s), first wait for even minute....\n");
	
	if (gpioInitialise() < 0) {
		fprintf(stderr,"gpio could not be initialized. \n");
		exit(-1);
	}
	
	gpioSetMode(13, PI_INPUT); 
	
	spi_handler = spiOpen(0, 250000, 49155);  //channel 0
	if (spi_handler < 0) {
		fprintf(stderr,"radioberry_protocol: spi bus could not be initialized. \n");
		exit(-1);
	}
	
	record_WSPR_Channels();

	if (spi_handler !=0)
		spiClose(spi_handler);

	gpioTerminate();
	
	return(0);
}

int startRecording() {
	
	time(&rawtime);
	info = gmtime(&rawtime );

	usleep(100000);
   
	if (info->tm_sec == 0) {
		if (info->tm_min % 2 == 0){
			//even minute, start recording...
			return 1;
		}
	}	
	return 0;
}

void record_WSPR_Channels(void) {

	int att = 0;
	int dither = 1; 
	int rando = 1;		// no function in firmware
	int sampleSpeed = 0;// no function in firmware

	int isample, qsample;
	float i_f_sample, q_f_sample;
	int nnrx = 0;
	int sample =0;
	
	FILE *fp;
	char *c2filename;
	float buffer[NRX][45000*2];

	while(1){
		while (!startRecording()) {}	//wait for even minute.
		
		printf("\nWSPR recording started %d min - %d sec \n", info->tm_min , info->tm_sec);
		
		memset(buffer, 0, sizeof(float) * 2 * 45000 * NRX);
		sample = 0;
		for (sample; sample < 45000; sample++){
			
			int lnrx = 0;
			for (lnrx; lnrx < NRX; lnrx++) {
				
				//the setting for the next read is passed to the firmware in the current read.
				nnrx = nnrx + 1; 								
				if (nnrx % NRX == 0)	{nnrx = 0;}			
				int freq = freqArray[nnrx];
				
				iqdata[0] = (((nnrx << 2) & 0x0C)  | (sampleSpeed & 0x03));
				iqdata[1] = (((rando << 6) & 0x40) | ((dither <<5) & 0x20) |  (att & 0x1F));
				iqdata[2] = ((freq >> 24) & 0xFF);
				iqdata[3] = ((freq >> 16) & 0xFF);
				iqdata[4] = ((freq >> 8) & 0xFF);
				iqdata[5] = (freq & 0xFF);
						
				spiXfer(spi_handler, iqdata, iqdata, 6);
				
				isample = (int)((signed char)iqdata[0]) << 16;
				isample += (int)((unsigned char)iqdata[1]) << 8;
				isample += (int)((unsigned char)iqdata[2]);
				qsample = (int)((signed char) iqdata[3]) << 16;
				qsample += (int)((unsigned char)iqdata[4]) << 8;
				qsample += (int)((unsigned char)iqdata[5]);
				
				i_f_sample = (float)isample /8388607.0 ;
				q_f_sample = (float)qsample /8388607.0;
				
				buffer[lnrx][sample * 2] = i_f_sample * GAIN;		
				buffer[lnrx][sample * 2 + 1] = q_f_sample * GAIN;
			
				while ( gpioRead(13) == 1) {}; // wait till rxFIFO buffer is filled with at least one element		
			}
		}
		
		printf("WSPR recording done, now writing to c2 files. \n");
		
		char date[12];
		char name[32];
		c2filename=malloc(sizeof(char)*15);
		int  type = 2;
		double  dialfreq;

		int i;
		for(i = 0; i < NRX; i++)
		{
			dialfreq = (freqArray[i] - 1500)/ 1000000.0;
			strftime(date, 12, "%y%m%d_%H%M", info);
			sprintf(name, "wspr_%d_%d_%s.c2", i, (int) dialfreq, date);
			strcpy(c2filename,"000000_0000.c2");
			if((fp = fopen(name, "wb")) == NULL)
			{
			  printf("c2 file could not be created/opened");
			  exit(-1);
			}
			fwrite(c2filename, 1, 14, fp);
			fwrite(&type, 1, 4, fp);
			fwrite(&dialfreq, 1, 8, fp);
			fwrite(buffer[i], 1, 360000, fp);
			fclose(fp);
		}
		printf("WSPR recordings written to c2 files \nThe WSPR c2 decoding will be started by starting decode.sh using crontab.\n");
	}
}