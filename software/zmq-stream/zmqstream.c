/* 
* Copyright (C)
* 2017, 2018, 2019 - Johan Maas, PA3GSB
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* This programs provide a IQ stream
*
*	http://www.pa3gsb.nl
*	  
*	Johan PA3GSB
*
*/

#include <stdlib.h>

#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <pigpio.h>

#include <sys/time.h>


struct timeval t20;
struct timeval t21;
int rx_count =0;

int freq = 1395000;
int att = 32;
int dither = 0;
int rando = 0;
int sampleSpeed = 0;
static int rx1_spi_handler;
unsigned char iqdata[6 * 65];

void rx1_spiReader(unsigned char iqdata[]) {
		
	iqdata[0] = (sampleSpeed & 0x03);
	iqdata[1] = (((rando << 6) & 0x40) | ((dither <<5) & 0x20) |  (att & 0x1F));
	iqdata[2] = ((freq >> 24) & 0xFF);
	iqdata[3] = ((freq >> 16) & 0xFF);
	iqdata[4] = ((freq >> 8) & 0xFF);
	iqdata[5] = (freq & 0xFF);
	for (int pointer = 6; pointer < 64 * 6; pointer+=6) memcpy(iqdata + pointer, iqdata, 6);	
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6 * 64);
	memcpy(iqdata + 6, iqdata, 64);
	// used to filter in subsriber...
	iqdata[0] = 'r';
	iqdata[1] = 'x';
	iqdata[2] = '0';
}


float timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

int main(int argc, char **argv)
{
	int major, minor, patch;
    zmq_version (&major, &minor, &patch);
    printf ("Current 0MQ version is %d.%d.%d\n", major, minor, patch);
	
	unsigned int rev = gpioHardwareRevision();  
	int rpi4 = (rev >= 10498321) ? 1 : 0;
	
	if (gpioInitialise() < 0) {
		fprintf(stderr,"hpsdr_protocol (original) : gpio could not be initialized. \n");
		exit(-1);
	}
	gpioSetMode(13, PI_INPUT); 	//rx1 samples
	gpioSetMode(20, PI_INPUT); 
	gpioSetMode(21, PI_OUTPUT);
	gpioWrite(21, 0);	//ptt off
	
	rx1_spi_handler = spiOpen(0, 9000000, 49155);  //channel 0
	if (rx1_spi_handler < 0) {
		fprintf(stderr,"radioberry_protocol: spi bus rx1 could not be initialized. \n");
		exit(-1);
	}

	//  Prepare our context and publisher
    void *context = zmq_ctx_new ();
    void *publisher = zmq_socket (context, ZMQ_PUB);
    //int rc = zmq_bind (publisher, "tcp://192.168.2.140:5556");
    int rc = zmq_bind (publisher, "tcp://169.254.45.200:5556");
	
	assert (rc == 0);

	gettimeofday(&t20, 0);

    while (1) {
		while (gpioRead(13) == 0) {usleep(30000);}//wait for enough samples
		
		// read 64 IQ samples.
		rx1_spiReader(iqdata);
		
		zmq_msg_t msg;
		rc = zmq_msg_init_size(&msg, 390);  //6 * 64 + 6 bytes subs filtering
		memcpy(zmq_msg_data(&msg), iqdata , 390);

        //s_send (publisher, iqdata);
		rc = zmq_msg_send(&msg, publisher, 0);
		//fprintf(stderr, "rc  %d\n", rc);
		if (rc == -1) {
			printf("%d\n", errno);
			break;
		}
		
		rx_count = rx_count + 64;
		if (rx_count == 48000) {
			rx_count = 0;
			gettimeofday(&t21, 0);
			float elapsed = timedifference_msec(t20, t21);
			fprintf(stderr, "48k executed in %f milliseconds.\n", elapsed);
			gettimeofday(&t20, 0);
		}
    }
    
	zmq_close (publisher);
    zmq_ctx_destroy (context);
	
	if (rx1_spi_handler !=0) spiClose(rx1_spi_handler);

	gpioTerminate();
	
	return 0;
}
