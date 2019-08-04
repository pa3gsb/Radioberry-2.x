/* 
* Copyright (C)
* 2018 - Johan Maas, PA3GSB
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
* Hermes Lite Emulator.
* 
* By using this emulator you have the possibility to connect to SDR programs like:
*	- linHPSDR
*	- pihpsdr
*	- Thetis
*
* SDR software packages supporting the 'HPSDR protocol-2' should be connected to the emulator.
*
*  This emulator works with the Radioberry radiocard plugged into a rpi-3.
*
*	http://www.pa3gsb.nl
*	  
*	2018 Johan PA3GSB
*
*/
#include "radioberry.h"
#include "local_audio_discovery.h"
#include "audio.h"
#include "mic.h"

#define SHORT_OPTIONS "lha:m:v"
#define HST_SHRT_OPTS ""

int use_local_audio_in = 0;
int use_local_audio_out = 0;

static struct option long_options[] =
{
	{"list-audio-devices",  0, 0, 'l'},
    {"audio-out",       	1, 0, 'a'},
	{"audio-in",        	1, 0, 'm'},
	{"help",            	0, 0, 'h'},
    {"version",         	0, 0, 'v'},
    {0, 0, 0, 0}
};

const char *version = "Radioberry hermeslite emulator version 08-28-2018";
const char *copyright =
    "Copyright (C) 2018 Johan Maas, PA3GSB\n"
    "This is free software; see the source for copying conditions.  There is NO\n"
    "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.";

int main(int argc, char **argv) {
	
	printIntroScreen();
	if (argc == 1) usage();
	
	audio_get_cards(); 
	
	while (1)
    {
		int c;
		int option_index = 0;
		c = getopt_long(argc,
						argv,
						SHORT_OPTIONS HST_SHRT_OPTS,
						long_options,
						&option_index);
		if (c == -1)
        {
            break;
        }
		
		switch (c)
		{
			case 'h':
				usage();
				exit(0);
			case 'v':
				print_version();
				exit(0);
			case 'l':
				discover_audio_cards();
				exit(0);
			case 'a':	
				if (!optarg)
				{
					usage();    /* wrong arg count */
					exit(1);
				}
				use_local_audio_out = 1;
				audio_open_output(selectAudioOutputDevice(atoi(optarg)));
				break;
			case 'm':	
				if (!optarg)
				{
					usage();    /* wrong arg count */
					exit(1);
				}
				use_local_audio_in = 1;
				audio_open_input(selectAudioInputDevice(atoi(optarg)));
				break;
			default:
				break;
		}
	}

	sem_init(&mutex, 0, 1);	
	sem_init(&mic_sample, 0, 0);
	sem_init(&high_priority_mutex, 0, 0);
	
	initialize_gpio();
	
	fprintf(stderr, "\n\nhermeslite protocol-2 emulator started \n\n");

	pthread_t pid1; 
	pthread_create(&pid1, NULL, handle_data_from_sdr_program, NULL);
	
	pthread_t pid2; 
	pthread_create(&pid2, NULL, send_high_priority_status_to_host, NULL);
	
	gettimeofday(&t1a, 0);
	if (use_local_audio_in==0) start_dummy_mic_thread(); else start_mic_thread();
	
	gettimeofday(&t1, 0);
	
	send_rx_iq_to_host();
	
	audio_close_output();
	
	fprintf(stderr, "hermeslite protocol-2 emulator stopped. \n");
}

void setup_isr_handler(int pin, gpioAlertFunc_t pAlert) {
  gpioSetMode(pin, PI_INPUT);
  gpioSetPullUpDown(pin,PI_PUD_UP);
  usleep(10000); // give time to settle to avoid false triggers
  gpioSetAlertFunc(pin, pAlert);
  gpioGlitchFilter(pin, HANDLER_STEADY_TIME_US);
}

void cw_ptt_alert(int gpio, int level, uint32_t tick) {
	fprintf(stderr, "cw - alert; cw_break_in = %d and cw-mode = %d\n", cw_break_in, cw);
    if (running && cw_break_in && cw){
		cw_ptt = level;
		sem_post(&high_priority_mutex);
	}
}

float timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

//int handle_data_from_sdr_program() {	
void *handle_data_from_sdr_program(void *arg) {
	int emulator_socket = -1;
	int saddr_len,buflen;
	struct sockaddr_in saddr;
	unsigned char* buffer = (unsigned char *)malloc(65536); 
	memset(buffer,0,65536);
	
	emulator_socket=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL)); 
	if(emulator_socket<0)
	{
		printf("error in socket\n");
		//return -1;
	}
	while(1)
	{
		saddr_len=sizeof saddr;
		buflen=recvfrom(emulator_socket,buffer,65536,0,(struct sockaddr *) &saddr,(socklen_t *)&saddr_len);
		if(buflen<0)
		{
			perror("hermeslite protocol-2 emulator stops unexpectedly");
			//return -1;
		}
		handle_packets_from_sdr_program(buffer,buflen);
	}
	close(emulator_socket);	
}	

int initialize_gpio() {
	if (gpioInitialise() < 0) {
		perror("hpsdr protocol-2: gpio could not be initialized. \n");
		exit(-1);
	}
	
	gpioSetMode(25, PI_INPUT); 	//data samples present
	gpioSetMode(23, PI_INPUT); 	//sample data[7]
	gpioSetMode(20, PI_INPUT);	//sample data[6]
	gpioSetMode(19, PI_INPUT);	//sample data[5]
	gpioSetMode(18, PI_INPUT);	//sample data[4]
	gpioSetMode(16, PI_INPUT);	//sample data[3]
	gpioSetMode(13, PI_INPUT);	//sample data[2]
	gpioSetMode(12, PI_INPUT);	//sample data[1]
	gpioSetMode(5, PI_INPUT); 	//sample data[0]
	gpioSetMode(6, PI_OUTPUT);  //rx1-pi-clk
	gpioSetMode(17, PI_OUTPUT); //rx2-pi-clk
	gpioSetMode(21, PI_OUTPUT); //ptt
	
	gpioWrite(21, 0); 			// set ptt off 
	gpioWrite(6, 0); 			// init rx1-pi-clk
	gpioWrite(17, 0); 			// init rx2-pi-clk
	
	rx1_spi_handler = spiOpen(0, 8500000, 49155);  //channel 0  15625000
	if (rx1_spi_handler < 0) { 
		perror("radioberry_protocol: spi bus rx1 could not be initialized. \n");
		exit(-1);
	}
	rx2_spi_handler = spiOpen(1, 8500000, 49155); 	//channel 1
	if (rx2_spi_handler < 0) {
		perror("radioberry_protocol: spi bus rx2 could not be initialized. \n");
		exit(-1);
	}
	//setup cw interrupt service routine;
	//keyer attached to fpga is touched=> set radioberry in tx mode. 
	//setup_isr_handler(17, cw_ptt_alert); 
	
	return 0;
}

void *send_high_priority_status_to_host(void *arg) {
	unsigned char* status_buffer = (unsigned char *)malloc(60); 
	memset(status_buffer,0,60);
	
	while(1) {
		if (running) {
			sem_wait(&high_priority_mutex);
			
			fprintf(stderr, "send high priority status to host. \n");
			
			status_buffer[0]=status_sequence>>24;
			status_buffer[1]=status_sequence>>16;
			status_buffer[2]=status_sequence>>8;
			status_buffer[3]=status_sequence;
			status_buffer[4]= (cw_ptt & 0x01);
			send_udp_packet(radioberry_socket, high_priority_addr, dst_addr, status_buffer, 60);
			status_sequence++;
		} else {usleep(20000);}
	}
}

//void *send_rx_iq_to_host(void *arg) {
void send_rx_iq_to_host() {
	unsigned char* iqbuffer_rx1 = (unsigned char *)malloc(2048); 
	memset(iqbuffer_rx1,0,2048);
	unsigned char* iqbuffer_rx2 = (unsigned char *)malloc(2048); 
	memset(iqbuffer_rx2,0,2048);
	unsigned char iqdata[6];
	unsigned int index_rx1 = 16;
	unsigned int index_rx2 = 16;

	iqbuffer_rx1[12] = 0x00; 	iqbuffer_rx2[12] = 0x00;	//MSB bitspersample
	iqbuffer_rx1[13] = 0x18; 	iqbuffer_rx2[13] = 0x18; 	//LSB bitspersample 24 bits / sample.
	iqbuffer_rx1[14] = 0x00; 	iqbuffer_rx2[14] = 0x00;	//MSB samplesperframe
	iqbuffer_rx1[15] = 0xEE; 	iqbuffer_rx2[15] = 0xEE;	//LSB samplesperframe 144-16 / 6  = 238 IQ samples
	
	while(1) {
		if (running) {
			if (gpioRead(25) == 1) {	
				int i =0;
				for (i; i< 64; i++){
					rx1_reader(iqdata);
					int j =0;
					for (j; j< 6; j++){
						iqbuffer_rx1[index_rx1++] = iqdata[j];
					}
					
					sem_post(&mic_sample); //sync mic stream
					
					if (index_rx1 >= 1444) {
						iqbuffer_rx1[0]=rx1_sequence>>24;
						iqbuffer_rx1[1]=rx1_sequence>>16;
						iqbuffer_rx1[2]=rx1_sequence>>8;
						iqbuffer_rx1[3]=rx1_sequence;

						send_udp_packet(radioberry_socket, src_addr[ddc[0]], dst_addr, iqbuffer_rx1, 1444);

						index_rx1 = 16;
						rx1_sequence++;		
					}
					if (nrx == 2) {
						rx2_reader(iqdata);
						int j =0;
						for (j; j< 6; j++){
							iqbuffer_rx2[index_rx2++] = iqdata[j];
						}

						if (index_rx2 >= 1444) {
							iqbuffer_rx2[0]=rx2_sequence>>24;
							iqbuffer_rx2[1]=rx2_sequence>>16;
							iqbuffer_rx2[2]=rx2_sequence>>8;
							iqbuffer_rx2[3]=rx2_sequence;
							
							send_udp_packet(radioberry_socket, src_addr[ddc[1]], dst_addr, iqbuffer_rx2, 1444);
							
							index_rx2 = 16;
							rx2_sequence++;
						}
					}
				}
			} else usleep(3000);
			
		} else {usleep(20000);}
	}
}

void handle_packets_from_sdr_program(unsigned char* buffer,int buflen) {
	int iphdrlen;
	struct iphdr *ip = (struct iphdr*)(buffer + sizeof (struct ethhdr));
	
	if (ip->protocol == 17) {
		iphdrlen =ip->ihl*4;
		struct udphdr *udp = (struct udphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));
		unsigned char * data = (buffer + iphdrlen  + sizeof(struct ethhdr) + sizeof(struct udphdr));
		int remaining_data = buflen - (iphdrlen  + sizeof(struct ethhdr) + sizeof(struct udphdr));
		
		switch (ntohs(udp->dest))  {
			case GENERAL_REGISTERS_FROM_HOST_PORT:
				if (radioberry_socket == -1) {
					struct sockaddr_in remaddr;	
					struct sockaddr_in sdr_addr;
					memset((char *)&sdr_addr, 0, sizeof(sdr_addr));
					sdr_addr.sin_addr.s_addr = ip->saddr;  
					memset((char *)&remaddr, 0, sizeof(remaddr));
					remaddr.sin_addr.s_addr = ip->daddr;  
					strcpy(radioberry_addr, inet_ntoa(remaddr.sin_addr));
					strcpy(sdr_client_addr, inet_ntoa(sdr_addr.sin_addr));
					remote_port = udp->source;
				}
				if (data[4] == 2) { 
					handleDiscovery(buffer);	
				} //else if (data[0] != 0xEF & data[1] != 0xFE && data[2] != 0x02 && data[4] == 0) {
					//fprintf(stderr,"GENERAL PACKET FROM HOST --- which info is important?\n");}
				break;
			case RECEIVER_SPECIFIC_REGISTERS_FROM_HOST_PORT:
				receiver_specific_registers_from_host_port(data);
				break;	
			case TRANSMITTER_SPECIFIC_REGISTERS_FROM_HOST_PORT:
				transmitter_specific_registers_from_host_port(data);
				break;
			case HIGH_PRIORITY_FROM_HOST_PORT:
				high_priority_from_host_port(data);
				break;
			case AUDIO_FROM_HOST_PORT:
				audio_write(data);
				break;
			case TX_IQ_FROM_HOST_PORT:
				tx_iq_from_host_port(data);
				break;
			default:
				break;
		}
	}
}

void receiver_specific_registers_from_host_port(unsigned char* data) {
	// the radioberry suports 2 ddc.
	int i = 0, ddc_no = 0, speed = 0, lnrx = 0;
	for (i=0; i <8; i++){
		if ( (data[7] >> i) & 1){
			ddc[ddc_no++] = i; lnrx++; 
			//fprintf(stderr, "ddc no %d \n", i); 
		}
	} 
	//fprintf(stderr,"Number of receivers: %d \n", lnrx);
	speed = data[18+(ddc[0]*6)] << 8 | data[19+(ddc[0]*6)];	
	sampleSpeed[0] = speed/48==1?0x00:speed/48==2?0x01:speed/48==4?0x02:speed/48==8?0x03:0x04;
	samplespeed = speed/48;
	//printf(stderr,"sample speed receiver[0] =  %d \n", sampleSpeed[0]);
	if (lnrx == 2){
		speed = data[18+(ddc[1]*6)] << 8 | data[19+(ddc[1]*6)];
		sampleSpeed[1] = speed/48==1?0x00:speed/48==2?0x01:speed/48==4?0x02:speed/48==8?0x03:0x04;
		//fprintf(stderr,"sample speed receiver[1] =  %d \n", sampleSpeed[1]);	
	}
	nrx = lnrx;
}

void transmitter_specific_registers_from_host_port(unsigned char* buffer) {		
   cw = ((buffer[5] & 0x02) == 0x02)? 1: 0;
   cw_iambic_mode  = ((buffer[5] & 0x28) == 0x28)? 2: ((buffer[5] & 0x08)==0x08)? 1: 0;
   cw_keyer_reverse=  ((buffer[5] & 0x04) == 0x04)? 1:0;
   cw_break_in = ((buffer[5] & 0x80) == 0x80)? 1:0;
   cw_keyer_speed  = buffer[9];
   cw_keyer_weight = buffer[10]; 	
}

void high_priority_from_host_port(unsigned char* data) {
			
	create_radioberry_socket();
	
	running = ((data[4] & 0x01) == 0x01)? 1:0;
	//fprintf(stderr," running %.2X %d \n",data[4], running);
	ptt = ((data[4] & 0x02) == 0x02) ? 1:0; 
	//fprintf(stderr," ptt %d \n",ptt);
	gpioWrite(21, ptt); 	// ptt 
	//data[5] [0] = CWX, [1] = Dot, [2] = Dash CW TODO!!
	int rxphase1, rxphase2, txphase;
	//rx 
	rxphase1 = data[9+(ddc[0]*4)] << 24 | data[10+(ddc[0]*4)] << 16  | data[11+(ddc[0]*4)] << 8 | data[12+(ddc[0]*4)] ;
	rxphase2 = data[9+(ddc[1]*4)] << 24 | data[10+(ddc[1]*4)] << 16  | data[11+(ddc[1]*4)] << 8 | data[12+(ddc[1]*4)] ;
	rxfreq1 =  (long) (((double) rxphase1 * 122880000.0) / 4294967296.0);
	rx1_phase = (uint32_t)ceil(((double)rxfreq1 * 4294967296.0 ) / 76800000);
	//fprintf(stderr,"rxfreq1 %d \n", rxfreq1);
	rxfreq2 =  (long) (((double) rxphase2 * 122880000.0) / 4294967296.0);
	rx2_phase = (uint32_t)ceil(((double)rxfreq2 * 4294967296.0 ) / 76800000);
	//fprintf(stderr,"rxfreq2 %d \n", rxfreq2);
	//tx
	txphase = ( data[329] << 24 | data[330] << 16  | data[331] << 8 | data[332] );
	txfreq =  (long) (((double) txphase * 122880000.0) / 4294967296.0);
	tx_phase = (uint32_t)ceil(((double)txfreq * 4294967296.0 ) / 76800000);
	//fprintf(stderr,"txfreq %d \n", txfreq);
	drive_level = data[345] & 0xFF;
	//if (prev_drive_level != drive_level) {
	//	prev_drive_level = drive_level;
	//	fprintf(stderr,"drive_level %d \n", drive_level);
	//}
	gain = ((int) ((signed char) data[1443])) + 12;
	//fprintf(stderr, "ad9866 gain = %d rx-gain = %d \n", gain, (gain -12));
	
	//TODO only sending if changed!
	spi_control_rx1_phase();
	spi_control_rx2_phase();
	spi_control_tx();
}

void create_radioberry_socket() {
	if (radioberry_socket == -1) {	
		int i = 0;
		for(i; i < 8; i++) {	
			src_addr[i].sin_family = PF_INET;
			src_addr[i].sin_port = htons(RX_IQ_TO_HOST_PORT_0 + i); 
			inet_aton(radioberry_addr, &src_addr[i].sin_addr);
		}
		dst_addr.sin_family = PF_INET;
		dst_addr.sin_port = htons(ntohs(remote_port));
		inet_aton(sdr_client_addr, &dst_addr.sin_addr);	
		
		//fprintf(stderr,"Destination IP-address %s  \n", inet_ntoa(dst_addr.sin_addr)); 
		//fprintf(stderr,"Destination Port %d \n", ntohs(dst_addr.sin_port));
		
		high_priority_addr.sin_family = PF_INET;
		high_priority_addr.sin_port = htons(HIGH_PRIORITY_TO_HOST_PORT);
		inet_aton(radioberry_addr, &high_priority_addr.sin_addr);
		
		mic_line_addr.sin_family = PF_INET;
		mic_line_addr.sin_port = htons(MIC_LINE_TO_HOST_PORT);
		inet_aton(radioberry_addr, &mic_line_addr.sin_addr);
		
		if((radioberry_socket = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0){
			perror("socket");
			exit(-1);
		}
		int optval = 1;
		if (setsockopt(radioberry_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
			fprintf (stderr, "Warning: no reuse\n");
		}
		if (setsockopt(radioberry_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
			fprintf (stderr, "Warning: no port reuse\n");
		}	
	}
}

void tx_iq_from_host_port(unsigned char* buffer) {
	
	unsigned char tx_iqdata[8];
	
	// protocol-2 the rate is 192ksps  (radioberry uses 48K; downsampling required!)
	// i and q (msb first using 24b/sample, radioberry uses only 16b/sample; using 2 msb bytes.) from index 4 to 1443 (240 i and q samples)
	
	if (!ptt) return;
	
	//getting the udp packages sometimes twice...?
	int seq = buffer[0] << 24 | buffer[1] << 16  | buffer[2]<< 8 | buffer[3] ;
	if (lseq == seq) return;
	lseq = seq;
	
	int index = 4;
	int sample = 0;
	for (sample=0; sample < 60; sample++) {
		tx_iqdata[0] = 0;	// todo eer support; also if eer support is on need to increase index step....
		tx_iqdata[1] = 0;
		tx_iqdata[2] = 0;
		tx_iqdata[3] = 0;
		tx_iqdata[4] = ((buffer[index++]) & 0xFF); 
		tx_iqdata[5] = ((buffer[index++]) & 0xFF); 
		tx_iqdata[6] = ((buffer[++index]) & 0xFF); 
		tx_iqdata[7] = ((buffer[++index]) & 0xFF); 
		sem_wait(&mutex);
		spiXfer(rx2_spi_handler, tx_iqdata, tx_iqdata, 8); //possible to return the fifo filled level....keep in sync! required?????
		sem_post(&mutex);
		int txbufferc = ((tx_iqdata[6] & 0x07) << 8) + (tx_iqdata[7] & 0xFF); 
		if (txbufferc > 2044)  usleep(100); 
		index = index + 20;	//decimation by 4
		
		lcount ++;
		if (lcount == 48000) {
			lcount = 0;
			gettimeofday(&t2, 0);
			float elapsd = timedifference_msec(t1, t2);
			printf("Code tx mode spi executed in %f milliseconds.\n", elapsd);
			gettimeofday(&t1, 0);
		}
	}
}

unsigned char l_mic_buffer[132]; 
void process_local_mic(unsigned char *mic_buffer){
	int i=0;
	
	if (running) {
		for(i=0;i<MIC_SAMPLES;i++) {
			l_mic_buffer[mic_count++] = mic_buffer[i*2+1]; //msb
			l_mic_buffer[mic_count++] = mic_buffer[i*2];
			
			/* making verbose function.....
			lcounta ++;
			if (lcounta == 48000) {
				lcounta = 0;
				gettimeofday(&t2a, 0);
				float elapsd = timedifference_msec(t1a, t2a);
				printf("Audio line-in executed in %f milliseconds.\n", elapsd);
				gettimeofday(&t1a, 0);
			}
			*/
			
		}
		l_mic_buffer[0]=mic_sequence>>24;
		l_mic_buffer[1]=mic_sequence>>16;
		l_mic_buffer[2]=mic_sequence>>8;
		l_mic_buffer[3]=mic_sequence;
		send_udp_packet(radioberry_socket, mic_line_addr, dst_addr, l_mic_buffer, 132);
		mic_sequence++;
		mic_count = 4;
	}
}

void *mic_dummy_thread(void *arg) {
	
	int i=0;
	unsigned char l_mic_buffer[132];
	
	while(1) {
		
		sem_wait(&mic_sample);
		i++;
		if ((i % samplespeed)==0) {
			
			i = 0;
		
			l_mic_buffer[mic_count++] = 0; //msb
			l_mic_buffer[mic_count++] = 0;
				
			if (mic_count == 132) {
				l_mic_buffer[0]=mic_sequence>>24;
				l_mic_buffer[1]=mic_sequence>>16;
				l_mic_buffer[2]=mic_sequence>>8;
				l_mic_buffer[3]=mic_sequence;
				send_udp_packet(radioberry_socket, mic_line_addr, dst_addr, l_mic_buffer, 132);
				mic_sequence++;
				mic_count = 4;
			}
		}
	}
	fprintf(stderr,"mic_dummy_thread: exiting\n");
	return NULL;
}

void start_dummy_mic_thread() {
	pthread_t pid1; 
	pthread_create(&pid1, NULL, mic_dummy_thread, NULL);
}


int createUDPSocket() {
	struct sockaddr_in myaddr;
	if ((discover_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("cannot create discover socket\n");
		return -1;
	}
	struct timeval timeout;      
    timeout.tv_sec = 0;
    timeout.tv_usec = TIMEOUT_MS;

	if (setsockopt(discover_socket, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0) {
		perror("setsockopt failed\n");
		return -1;
	}
	int optval = 1;
    setsockopt(discover_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    setsockopt(discover_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
		
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = PF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	myaddr.sin_port = htons(SERVICE_PORT);

	if (bind(discover_socket, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind to discover socket failed");
		return -1;
	}
	return 0;
}

void fillDiscoveryReplyMessage() {
	memset(broadcastReply, 0, sizeof(broadcastReply));
	int i = 5;
	for (i; i < 11; i++) broadcastReply[i] = i - 5; //MAC 00-01-02-03-04-05
	broadcastReply[4]  = 0x02;
	broadcastReply[11] = HERMESLITE;	
	broadcastReply[13] = FIRMWARE_VERSION;	
	broadcastReply[20] = MAX_RECEIVERS;
	broadcastReply[36] = NR_ADC;
	broadcastReply[39] = MAX_RECEIVERS;
}

int handleDiscovery(unsigned char* buffer) {
	int iphdrlen;
	struct sockaddr_in remaddr;	
	socklen_t addrlen = sizeof(remaddr);
	if (running ==  0) {
		if (radioberry_socket != -1) close(radioberry_socket);
		radioberry_socket = -1;
	} else return 0;
	
	struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));
	iphdrlen =ip->ihl*4;
	struct udphdr *udp = (struct udphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));   
	struct sockaddr_in source,dest;	 
	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = ip->saddr;     
	memset(&dest, 0, sizeof(dest));
	dest.sin_addr.s_addr = ip->daddr;   
	memset((char *)&remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = PF_INET;
	remaddr.sin_addr.s_addr = ip->saddr;  
	remaddr.sin_port = udp->source;
	
	fprintf(stderr,"Discovery packet received \n");
	fprintf(stderr,"SDR Program IP-address %s  \n", inet_ntoa(remaddr.sin_addr)); 
	fprintf(stderr,"Discovery Port %d \n", ntohs(remaddr.sin_port));

	//strcpy(sdr_client_addr, inet_ntoa(remaddr.sin_addr));	
	fillDiscoveryReplyMessage();
	createUDPSocket();			
	if (sendto(discover_socket, broadcastReply, sizeof(broadcastReply), 0, (struct sockaddr *)&remaddr, addrlen) < 0) {
		perror("Discovery reply sendto error");
		return -1;
	}
	close(discover_socket);
	return 0;
}

void usage(void)
{
    printf("Usage: radioberry [OPTION]... [COMMAND]...\n"
           "COMMANDs to initialize the radioberry firmware.\n\n");

    printf(
		" -a, --audio-out=ID\t select audio output device number. See model list\n"
        " -m, --audio-in=ID\t select audio input device. See model list\n"
        " -l, --list \t\t list all audio device and exit\n"
        " -h, --help \t\t display this help and exit\n"
        " -v, --version \t\t output version information and exit\n\n"
    );

    printf("\nReport bugs to <pa3gsb@gmail.com>.\n");
}

void print_version(void){
	printf("Version information: \n%s \n\n", version);
	printf("%s \n", copyright);
}

void printIntroScreen() {
	fprintf(stderr,"\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr, "\tRadioberry V2.0 beta 2 and beta 3 (incl. small mod).\n");
	fprintf(stderr,	"\n");
	fprintf(stderr, "\tSupporting:\n");
	fprintf(stderr, "\t\t - openhpsdr protocol-2.\n");
	fprintf(stderr, "\t\t - full duplex mode.\n");
	fprintf(stderr, "\t\t - 2rx slices max 192K sampling rate\n");
	fprintf(stderr,	"\n\n");
	fprintf(stderr, "\t\t\t Have fune Johan PA3GSB\n");
	fprintf(stderr, "\n\n");
	fprintf(stderr, "\n\tReport bugs to <pa3gsb@gmail.com>.\n");
	fprintf(stderr, "====================================================================\n");
	fprintf(stderr, "====================================================================\n");
}

void spi_control_rx1_phase() {
		
	unsigned char iqdata[6];
	
	iqdata[0] = (0x10 | (sampleSpeed[0] & 0x03));
	iqdata[1] = (~(gain & 0x2F));
	iqdata[2] = ((rx1_phase >> 24) & 0xFF);
	iqdata[3] = ((rx1_phase >> 16) & 0xFF);
	iqdata[4] = ((rx1_phase >> 8) & 0xFF);
	iqdata[5] = (rx1_phase & 0xFF);
			
	sem_wait(&mutex);			
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
	sem_post(&mutex);
}

void spi_control_rx2_phase() {
		
	unsigned char iqdata[6];
	
	iqdata[0] = (0x20 | (sampleSpeed[1] & 0x03));
	iqdata[1] = (~(gain & 0x2F));
	iqdata[2] = ((rx2_phase >> 24) & 0xFF);
	iqdata[3] = ((rx2_phase >> 16) & 0xFF);
	iqdata[4] = ((rx2_phase >> 8) & 0xFF);
	iqdata[5] = (rx2_phase & 0xFF);
			
	sem_wait(&mutex);			
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
	sem_post(&mutex);
}

void spi_control_tx() {
	
	unsigned char iqdata[6];
	
	iqdata[0] = 0x30;
	iqdata[1] = drive_level / 6.4;  // convert drive level from 0-255 to 0-39 )
	iqdata[2] = ((tx_phase >> 24) & 0xFF);
	iqdata[3] = ((tx_phase >> 16) & 0xFF);
	iqdata[4] = ((tx_phase >> 8) & 0xFF);
	iqdata[5] = (tx_phase & 0xFF);
		
	sem_wait(&mutex);		
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
	sem_post(&mutex);
}

int level = 0;
void rx1_reader(unsigned char iqdata[]){
	int i = 0;
	for (i; i < 6 ; i++) {
		level = !level;
		gpioWrite(6, level);
		
		uint32_t value = gpioRead_Bits_0_31();
		value = gpioRead_Bits_0_31();
		
		iqdata[i]  =  (((value >> 23) & 1) << 7);
		iqdata[i] |=  (((value >> 20) & 1) << 6);
		iqdata[i] |=  (((value >> 19) & 1) << 5);
		iqdata[i] |=  (((value >> 18) & 1) << 4);
		iqdata[i] |=  (((value >> 16) & 1) << 3);
		iqdata[i] |=  (((value >> 13) & 1) << 2);
		iqdata[i] |=  (((value >> 12) & 1) << 1);
		iqdata[i] |=  (((value >> 5) & 1));
	}
}

int level2 = 0;
void rx2_reader(unsigned char iqdata[]){
	int i = 0;
	for (i; i < 6 ; i++) {
		level2 = !level2;
		gpioWrite(17, level2);
		
		uint32_t value = gpioRead_Bits_0_31();
		value = gpioRead_Bits_0_31();
		
		iqdata[i]  =  (((value >> 23) & 1) << 7);
		iqdata[i] |=  (((value >> 20) & 1) << 6);
		iqdata[i] |=  (((value >> 19) & 1) << 5);
		iqdata[i] |=  (((value >> 18) & 1) << 4);
		iqdata[i] |=  (((value >> 16) & 1) << 3);
		iqdata[i] |=  (((value >> 13) & 1) << 2);
		iqdata[i] |=  (((value >> 12) & 1) << 1);
		iqdata[i] |=  (((value >> 5) & 1));
	}
}

// end of source.