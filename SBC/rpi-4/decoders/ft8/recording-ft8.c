/*
	FT8 recording

	
	Johan Maas PA3GSB
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <fftw3.h>
#include <time.h>
#include <complex.h>
#include <limits.h>
#include <assert.h>

//#include <zmq.h>

#include "radioberry_ioctl.h"

#define NRX 4				// FOR NOW MAX = 4 number of receivers.

int freqArray[8] = {3574500, 5358500, 7075500, 10137500,  14075500, 18101500,  21075500, 1841500  };
//int freqArray[8] = {1841500, 3574500, 5358500, 7075500, 10137500,  14075500, 18101500,  21075500  };


struct rb_info_arg_t rb_info;
int fd_rb;
int stopDecoding = 0;

u_int32_t commands[256];

//void *context;
//void *publisher;
//int rc;

//complex float* iqdata;
//int iqdata_count = 0;

complex float *iq_buffer;


void handle_sigint(int sig) 
{ 
	stopDecoding = 1;
}

void send_control(unsigned char command) {
	
	u_int32_t command_data = commands[command];
	
	rb_info.rb_command = 0x01;
	rb_info.command = command;
	rb_info.command_data = command_data;
	
	//fprintf(stderr, "RB-Command = %02X Command = %02X  command_data = %08X\n", rb_info.rb_command, command, command_data);
	
	if (ioctl(fd_rb, RADIOBERRY_IOC_COMMAND, &rb_info) == -1) {
		fprintf(stderr, "Could not sent commando to radioberry device.");
	}
}

int initDecoder() 
{
	if((fd_rb = open("/dev/radioberry", O_RDWR)) < 0){
		perror("Failed to open /dev/radioberry");
		exit(-1);
	}
	
	rb_info.rb_command = 0x00; 
	
	//required to retrieve gateware information.
	if (ioctl(fd_rb, RADIOBERRY_IOC_COMMAND, &rb_info) == -1) {
		fprintf(stderr, "RADIOBERRY_IOC_COMMAND Error.");
		exit(-1);		
	} 
	fprintf(stderr, "Radioberry gateware version %d-%d.\n", rb_info.major, rb_info.minor);


//	context = zmq_ctx_new ();
//    publisher = zmq_socket (context, ZMQ_PUB);
//    int lrc = zmq_bind (publisher, "tcp://192.168.2.140:5556");
    //int lrc = zmq_bind (publisher, "tcp://169.254.45.200:5556");
//	assert (lrc == 0);
	
//	iqdata = malloc(sizeof(complex float)*4000); 

	memset(commands,0,256); // initialise the commands.	

	commands[0x00] = 0x0000001C;        //8rx=3C   4rx=1C
	commands[0x02] = freqArray[0];		//f1
	commands[0x04] = freqArray[0];		//f1
	commands[0x06] = freqArray[1];		//f2
	commands[0x08] = freqArray[2];		//f3
	commands[0x0A] = freqArray[3];		//f4		
	commands[0x0C] = freqArray[4];		//f5		
	commands[0x0E] = freqArray[5];		//f6		
	commands[0x10] = freqArray[6];		//f7		
	commands[0x24] = freqArray[7];		//f8	
	
	commands[0x14] = 0x0000005f;		//att
	 
	send_control(0x00);
	send_control(0x02);

	send_control(0x04);
	send_control(0x06);
	send_control(0x08);
	send_control(0x0A);
	send_control(0x0C);
	send_control(0x0E);
	send_control(0x10);
	send_control(0x24);
	
	send_control(0x14);
	
	return 0;
}

int closeDecoder() {
	fprintf(stderr, "exiting decoding process. \n");
	if (fd_rb != 0) close(fd_rb);
	
//	zmq_close (publisher);
//    zmq_ctx_destroy (context);
	
//	free(iqdata);
	free(iq_buffer);
	return 0;
}

void ft8_decode(void)
{
	int sample = 0;
	int iq_count =0;
	char rx_buffer[512];
	int nr_samples = 0;
	time_t rawtime;
	struct tm *info;
	FILE *fp;
	char *c2filename;
	int nr_samples_bytes = -1;
	int sample_byte_index = 0;
	
	fprintf(stderr, "\nFT8-decoding process... \n");
	
	iq_buffer = malloc(sizeof(complex float)*240000 * NRX);

	while (!stopDecoding) {
	   
	   if (sample ==0) {
			//wait till 59 sec in a minute are passed.
			struct timespec t;
			clock_gettime(CLOCK_REALTIME, &t);
			t.tv_sec = 58 - t.tv_sec % 60;
			t.tv_nsec = 999999999L - t.tv_nsec;
			nanosleep(&t, NULL);
			
			time(&rawtime);
			info = gmtime(&rawtime );
			
			sample_byte_index = 0;
			nr_samples_bytes = -1;
	   }
	
		if (sample_byte_index >= nr_samples_bytes) {
			sample_byte_index = 0;
			nr_samples_bytes = read(fd_rb , rx_buffer , 72);
		}
		
		int lnrx = 0;
		for (lnrx; lnrx < NRX; lnrx++) {
	
			int left_sample, right_sample;
		
			left_sample   = (int)((signed char) rx_buffer[sample_byte_index++])<<16;
			left_sample  |= (int)((((unsigned char)rx_buffer[sample_byte_index++])<<8)&0xFF00);
			left_sample  |= (int)((unsigned char)rx_buffer[sample_byte_index++]&0xFF);
			right_sample  = (int)((signed char)rx_buffer[sample_byte_index++]) << 16;
			right_sample |= (int)((((unsigned char)rx_buffer[sample_byte_index++])<<8)&0xFF00);
			right_sample |= (int)((unsigned char)rx_buffer[sample_byte_index++]&0xFF);
			
			iq_buffer[sample + (lnrx * 240000)] = CMPLXF( 100 * ((float)left_sample/8388607.0), -100 * ((float)right_sample/8388607.0) );
				
			//if (lnrx==0) iqdata[iqdata_count] = CMPLXF( ((float)left_sample/8388607.0), ((float)right_sample/8388607.0) );
		}

		//iqdata_count ++;
		//if (iqdata_count == 4000) {
		//	iqdata_count = 0;

		//	zmq_msg_t msg;
		//	rc = zmq_msg_init_size(&msg, sizeof(complex float) * 4000);
		//	memcpy(zmq_msg_data(&msg), iqdata , sizeof(complex float) * 4000);
		//	rc = zmq_msg_send(&msg, publisher, 0);
		//	if (rc == -1) fprintf(stderr, "zmq send failure, error: %d\n", rc);
		//}
		
		sample++;	
		
		if (sample % 4000 == 0) fprintf(stderr, ".");
	
		if (sample == 236000) {

			fprintf(stderr, "\nFT8 recording done, now writing to c2 files.... \n");
			
			char date[12];
			char name[32];
			double  dialfreq;

			int i = 0;
			for(i; i < NRX; i++)
			{
				dialfreq = freqArray[i] * 1.0; 
				strftime(date, 12, "%y%m%d_%H%M", info);
				sprintf(name, "./recordings/ft8_%d_%d_%s.c2", i, (int) dialfreq, date);
				if((fp = fopen(name, "wb")) == NULL)
				{
				  printf("c2 file could not be created/opened");
				  exit(-1);
				}
				fwrite(&dialfreq, 1, 8, fp);
				fwrite(&iq_buffer[i * 240000], 8, 240000, fp);
				fclose(fp);
			
			}
			fprintf(stderr,"FT8 recordings written to c2 files. \n");
			
			sample = 0;
		}
    }
	
	closeDecoder();
}

int main(int argc,char **argv)
{
	if (initDecoder()) exit (-1);
	signal(SIGINT, handle_sigint);	
	ft8_decode();

   return 0;
}