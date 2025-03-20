/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>


* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* This programs is the c part of the firmware for RadioBerry Juice. 
*
* The belonging gateware can be found at: https://github.com/softerhardware/Hermes-Lite2
*
*
* By using this program you have the possibility to connect to SDR programs like:
*	- pihpsdr (including TCP)
*	- Quisk
*	- PSDR
*	- SPARK
*	- SDRConsole
*
*	Using the 'HPSDR UDP protocol'; also called protocol-1
*
*
*	http://www.pa3gsb.nl
*	  
*	Johan PA3GSB
*
*/

#include "radioberry.h"
#include "gateware.h"
#include "stream.h"
#include "register.h"
#include "pa.h"

#ifndef _WIN32	
#include "sensors.h"
#endif

#ifdef _WIN32
	#define FIRMWAREVERSION "W-J-03-20-2025"
#else
	#define FIRMWAREVERSION "L-J-03-20-2025"
#endif

void printIntroScreen() {
	fprintf(stderr,"\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr, "\t\t\tRadioberry V2.0\n\n\n");
	fprintf(stderr,	"\tSupports 6 receivers and 1 transmitter.\n\n");
	fprintf(stderr, "\tBuild version: %s\n\n", FIRMWAREVERSION);
	fprintf(stderr, "\tHave fun Johan PA3GSB\n\n");
	fprintf(stderr, "\tReport requests or bugs to <pa3gsb@gmail.com>.\n");
	fprintf(stderr, "====================================================================\n");
	fprintf(stderr, "====================================================================\n");
}

void handle_sigint(int sig) 
{ 
	if (running) fprintf(stderr, "  SDR program is still running; please stop SDR first.\n");
	closerb = 1;
}

int load_radioberry_gateware() {
	
	int ret = 0;
	
	time_t gstartTime = clock();
	
	ret = load_gateware_image_into_fpga();

	if (ret < 0) { return ret; };

	time_t gstopTime = clock();
	double gsecondsElapsed = (double)(gstopTime - gstartTime) / CLOCKS_PER_SEC;
	
	fprintf(stderr, "Radioberry gateware loaded taking %0.1f seconds.\r\n", gsecondsElapsed);
	
	return ret;
}

static void *rb_register_thread(void *arg) {
	sprintf(gatewareversion,"%d.%d", gateware_major_version, gateware_minor_version);
	sprintf(firmwareversion,"%s", FIRMWAREVERSION);
	//major-minor-build of ftdi driver
	sprintf(driverversion,"%02x.%02x.%02x", ((driver_version >>16) & 0xFF), ((driver_version >>8) & 0xFF), (driver_version & 0xFF));
	gateware_fpga_type == 0 ? sprintf(fpgatype,"%s", "-") : gateware_fpga_type == 1 ? sprintf(fpgatype,"%s", "CL016") : sprintf(fpgatype,"%s", "CL025");
	registerRadioberry();
	return NULL;
}

void start_rb_register_thread() {
	pthread_t pid1; 
	pthread_create(&pid1, NULL, rb_register_thread, NULL);
}

static void *rb_pa_thread(void *arg) {
	fprintf(stderr,"Try to connect to external Amplifier\n");
	initAmplifier();
	while(1) {
		if (!isAmplifierConnected()) if (connectToAmplifier()< 0) break; else exhangeAmplifierInfo();
		if (!isAmplifierConnected()) usleep(1000000); else usleep(10000);
		if (!running) setAmplifierInfo();
	}
	disconnectAmplifier();
	return NULL;
}

void start_rb_pa_thread() {
	pthread_t pid; 
	pthread_create(&pid, NULL, rb_pa_thread, NULL);
}

uint16_t recalculateADCValue(uint16_t adc_in) {
	// need to recalculate the AMP measurement for HL2 type of radio
	float bridge_volt = 0.018139; float refvoltage = 3.3f; int adc_cal_offset = 0;
	float volts = (float)((adc_in - adc_cal_offset) / 4095.0f * refvoltage);
	if (volts < 0) volts = 0;
	float watts = (float)(pow(volts, 2) / bridge_volt);
	// having the power ; recalculate the adc value for hermes lite type SDR model.
	bridge_volt = 1.8f;	refvoltage = 3.3f; adc_cal_offset = 3;
	uint16_t pa_dac = (uint16_t)(sqrt(watts * bridge_volt) * 4095.0f / refvoltage) + adc_cal_offset;
	return pa_dac;
}

void getStreamAndSendPacket() {
	
	if (read_stream(hpsdrdata) < 0) return; // fetching 1032 bytes of ethernet packet.
	// the sdr data coming from the radioberry need to be enhanced with adddional information.
	if (isAmplifierConnected()) {
		if (!pa_temp_ok && (((hpsdrdata[11] & 0x01) == 0x01) | ((hpsdrdata[523] & 0x01) == 0x01))) {
			//radio in tx mode ; switch back to rx.
			hpsdrdata[11] = hpsdrdata[11] & 0xFE; hpsdrdata[523] = hpsdrdata[523] & 0xFE;
		} 
		// AMP is connected; info of the AMP will be used to present to the operator.
		if ((hpsdrdata[11] & 0xF8) == 0x08) {
			uint16_t pa_dac = recalculateADCValue(pa_fwd_dac);
			//fprintf(stderr,"pwr fwd dac %d recalc %d\n", pa_fwd_dac, pa_dac);
			hpsdrdata[14] 	= ((pa_dac >> 8) & 0xFF); hpsdrdata[15] = (pa_dac & 0xFF); 
			int protocol_temp = (int) (4096/3.26) * ((pa_temp/ 100.0) + 0.5);
			hpsdrdata[12] =  ((protocol_temp >> 8) & 0xFF); hpsdrdata[13] = (protocol_temp & 0xFF);
		} else if ((hpsdrdata[11] & 0xF8) == 0x10) {
			uint16_t pa_dac = recalculateADCValue(pa_rev_dac);
			//fprintf(stderr,"pwr rev dac %d recalc %d\n", pa_rev_dac, pa_dac);
			hpsdrdata[12] = ((pa_dac >> 8) & 0xFF); hpsdrdata[13] = (pa_dac & 0xFF); 
			//current
			hpsdrdata[14] = 0x00; hpsdrdata[15] = 0x00; 
		}
		if ((hpsdrdata[523] & 0xF8) == 0x08) {
			uint16_t pa_dac = recalculateADCValue(pa_fwd_dac);
			hpsdrdata[526] 	= ((pa_dac >> 8) & 0xFF); hpsdrdata[527] = (pa_dac & 0xFF); 
			int protocol_temp = (int) (4096/3.26) * ((pa_temp/ 100.0) + 0.5);
			hpsdrdata[524] =  ((protocol_temp >> 8) & 0xFF); hpsdrdata[525] = (protocol_temp & 0xFF);
		}  else if ((hpsdrdata[523] & 0xF8) == 0x10)  {
			uint16_t pa_dac = recalculateADCValue(pa_rev_dac); 
			hpsdrdata[524] = ((pa_dac >> 8) & 0xFF); hpsdrdata[525] = (pa_dac & 0xFF); 
			//current
			hpsdrdata[526] = 0x00; hpsdrdata[527] = 0x00;
		}
	} else {		
		//check temperature from gateware. if no i2c device data from gateware; applying computer temperature 	
		if (((hpsdrdata[11] & 0x08) == 0x08) || ((hpsdrdata[523] & 0x08) == 0x08)) { 
			if ( ((hpsdrdata[12] == 0x0F | hpsdrdata[524] == 0x0F)) && ((hpsdrdata[13] == 0xFF | hpsdrdata[525] == 0xFF)) ){
				hpsdrdata[12]  =  ((sys_temp >> 8) & 0xFF); hpsdrdata[13]  = (sys_temp & 0xFF);
				hpsdrdata[524] =  ((sys_temp >> 8) & 0xFF); hpsdrdata[525] = (sys_temp & 0xFF);
			}				
		}
		
		//check current from gateware. if no i2c device data from gateware is 0x0FFF; beter default data is 0 current. 	
		if (((hpsdrdata[11] & 0x10) == 0x08) || ((hpsdrdata[523] & 0x10) == 0x08)) {
			if ( ((hpsdrdata[12] == 0x0F | hpsdrdata[524] == 0x0F)) && ((hpsdrdata[13] == 0xFF | hpsdrdata[525] == 0xFF)) ){
				hpsdrdata[14]  = 0x00; hpsdrdata[15]  = 0x00;
				hpsdrdata[526] = 0x00; hpsdrdata[527] = 0x00;
			}				
		}
		
		
	}
	
	if (sock_TCP_Client >= 0) {
		if (sendto(sock_TCP_Client, (const char*) hpsdrdata, sizeof(hpsdrdata), 0, NULL, 0) != 1032) fprintf(stderr, "TCP send error");
	} else {
		if (sendto(fd, (const char*) hpsdrdata, sizeof(hpsdrdata), 0, (struct sockaddr *)&remaddr, addrlen) != 1032) fprintf(stderr, "UDP send error");
	}
}

void runRadioberry() {
	fprintf(stderr, "Radioberry, Starting packet tx part. \n");
	while(!(running==0 && closerb == 1)) {
		if (running) {
			active = 1;
			getStreamAndSendPacket();
		} else {active = 0; usleep(20000); }
	}
}

void *packetreader(void *arg) {
	int size, bytes_read, bytes_left;
	unsigned char buffer[1032];
	
	memset(buffer, 0, 1032);
	write_stream(buffer);
	write_stream(buffer);
	
	uint32_t *code0 = (uint32_t *) buffer; 
	fprintf(stderr, "Radioberry, Starting packet rx part. \n");
	while(1) {
		
		if (sock_TCP_Client >= 0) {
			// handle TCP protocol.
			bytes_read=0;
			bytes_left=1032;
			while (bytes_left > 0) {
				size = recvfrom(sock_TCP_Client, (char*) buffer+bytes_read, (size_t) bytes_left, 0, NULL, 0);
				if (size < 0 && errno == EAGAIN) continue;
				if (size < 0) break;
				bytes_read += size;
				bytes_left -= size;
			}
			if (bytes_read == 1032) handlePacket((unsigned char*) buffer); //else fprintf(stderr, "tcp packet received; wrong length %d \n", bytes_read);
		} 
		else {
			// handle UDP protocol.
			int recvlen = recvfrom(fd, (char*) buffer, sizeof(buffer), 0, (struct sockaddr *)&remaddr, &addrlen);
			if (recvlen > 0) { udp_retries = 0; handlePacket((unsigned char*) buffer); } else udp_retries++;
			// If nothing has arrived via UDP for some time (defined by socket timeout * 10) , try to open TCP connection.
			if (sock_TCP_Client < 0 && udp_retries > 10)
			{
				if((sock_TCP_Client = accept(sock_TCP_Server, NULL, NULL)) > -1)
				{
						fprintf(stderr, "sock_TCP_Client: %d connected to sock_TCP_Server: %d\n", sock_TCP_Client, sock_TCP_Server);
				}
				udp_retries=0;
			}
		}
	}
}

void handlePacket(unsigned char* buffer){
	uint32_t code;
	memcpy(&code, buffer, 4);
	switch (code)
	{
		default:
			{
			fprintf(stderr, "Received packages not for me! \n");
			}
			break;
		case 0x0002feef:
			{
			fprintf(stderr, "Discovery packet received \n");
			fprintf(stderr,"SDR Program IP-address %s  \n", inet_ntoa(remaddr.sin_addr)); 
			fprintf(stderr, "Discovery Port %d \n", ntohs(remaddr.sin_port));
			memset(broadcastReply, 0, 60);
			unsigned char reply[22] = {0xEF, 0xFE, 0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, gateware_major_version, HERMESLITE, 0, 0, 0, 0, 0, 0, 0, 0, NR, 0, gateware_minor_version };
			memcpy(broadcastReply, reply, 22);
			if (sock_TCP_Client > -1) {
				send(sock_TCP_Client, (const char*) broadcastReply, 60, 0);
				close(sock_TCP_Client);
				sock_TCP_Client = -1;
				}
			else if (sendto(fd, (const char*) broadcastReply, sizeof(broadcastReply), 0, (struct sockaddr *)&remaddr, addrlen) < 0) fprintf(stderr, "broadcast reply error");
			}
			break;
		case 0x0004feef:
			{
			fprintf(stderr, "SDR Program sends Stop command \n");
			running = 0;
			while (active) usleep(1000);
			if (sock_TCP_Client > -1)
			{
				close(sock_TCP_Client);
				sock_TCP_Client = -1;
				fprintf(stderr, "SDR Program sends TCP Stop command \n");
			} else fprintf(stderr, "SDR Program sends UDP Stop command \n"); 
			}
			write_rb_stream(buffer);
			break;
		case 0x0104feef:
		case 0x0304feef:
			{
			fprintf(stderr, "Start Port %d \n", ntohs(remaddr.sin_port));
			running = 1;
			if (sock_TCP_Client > -1)
				fprintf(stderr, "SDR Program sends TCP Start command \n");
			else
				fprintf(stderr, "SDR Program sends UDP Start command \n");
			}
			write_rb_stream(buffer);
			break;
		case 0x1104feef: 
			{
			fprintf(stderr, "Connect the TCP client to the server\n");
			if (sock_TCP_Client < 0)
			{
				if((sock_TCP_Client = accept(sock_TCP_Server, NULL, NULL)) < 0)
				{
					fprintf(stderr, "*** ERROR TCP accept ***\n");
					perror("accept");
					return;
				}
				fprintf(stderr, "sock_TCP_Client: %d connected to sock_TCP_Server: %d\n", sock_TCP_Client, sock_TCP_Server);
				running = 1;
				fprintf(stderr, "SDR Program sends TCP Start command \n");
			}	
			}
			write_rb_stream(buffer);
			break;
		case 0x0201feef:
			{
			write_rb_stream(buffer);
			}
			break;		
	}
}

void write_rb_stream(unsigned char* buffer) {
	setAmplifierInfoBuffer(buffer);  
	write_stream(buffer);
}


#ifndef _WIN32

void read_temperature_raspberryPi() {
	FILE *thermal;
	thermal = fopen("/sys/class/thermal/thermal_zone0/temp","r");
	float systemp, millideg;
	fscanf(thermal,"%f",&millideg);
	systemp = millideg / 1000;
	//fprintf(stderr, "CPU temperature is %f degrees C\n",systemp);
	sys_temp = (int) (4096/3.26) * ((systemp/ 100.0) + 0.5);
	//fprintf(stderr, "CPU temperature in protocol has value %x\n",sys_temp);
	fclose(thermal);
}


int read_temperature_linux() {
	double value;
	FILE *thermal;
	thermal = fopen("/sys/class/hwmon/hwmon0/device/temp","r");
	
	 if (thermal) {
		int res, err = 0;
		errno = 0;
		res = fscanf( thermal, "%lf", &value);
		if ( res == EOF && errno == EIO)
			err = -SENSORS_ERR_IO;
		else if ( res != 1)
			err = -SENSORS_ERR_ACCESS_R;
		res = fclose( thermal);
		if ( err)
			return err;

		if ( res == EOF) {
			if ( errno == EIO)
				return -SENSORS_ERR_IO;
			else
				return -SENSORS_ERR_ACCESS_R;
		}
		value /= get_type_scaling( SENSORS_SUBFEATURE_TEMP_INPUT);
		sys_temp = (int) (4096/3.26) * ((value/ 100.0) + 0.5);
		//fprintf(stderr, "CPU temperature in protocol has value %x\n",sys_temp);
	 }
	 
	 return 0;
}

void isRPILinux(void)
{
	struct utsname buffer;
	errno = 0;
	if (uname(&buffer) < 0) {fprintf(stderr, "Could not find sys info\n"); return;}
	if (!strncasecmp("Linux", buffer.sysname, 5) & !strncasecmp("raspberrypi", buffer.nodename, 11)) rpiLinux =1;
}
#endif

static void *temperature_thread(void *arg) {
	while(1) {
		usleep(1000000);
		//determine temp every 1 sec.

#ifndef _WIN32			
		if (rpiLinux) read_temperature_raspberryPi(); else read_temperature_linux();
#else
		//for windows the temp is set hard to 42 .... todo.
		sys_temp = (int) (4096.0/3.26) * ((42/ 100.0) + 0.5);
		//fprintf(stderr, "Win CPU temperature in protocol has value %d\n",sys_temp);
#endif
	}
	fprintf(stderr,"timer_thread: exiting\n");
	return NULL;
}

void start_temperature_thread() {
	pthread_t pid1; 
	pthread_create(&pid1, NULL, temperature_thread, NULL);
}


int main(int argc, char **argv)
{	
	printIntroScreen();

#ifndef _WIN32	
	isRPILinux();
#endif

	if (load_radioberry_gateware() < 0) {
		fprintf(stderr,"Radioberry; loading radioberry gateware failed. \n");
		return -1;
	}
	
	driver_version = getFirmwareVersion();
	//fprintf(stderr, "%06x\n", driver_version);
	
	int ret;
	if (ret = init_stream() < 0) { return ret; }	
	
#ifdef _WIN32	
	WSADATA wsa;
	
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}
#endif
	
	pthread_t pid1; 
	pthread_create(&pid1, NULL, packetreader, NULL); 
	
	/* create a UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("cannot create socket %d\n", fd);
		perror("cannot create socket\n");
		return 0;
	}
	struct timeval timeout;      
    timeout.tv_sec = 0;
    timeout.tv_usec = TIMEOUT_MS;
	int wtimeout = TIMEOUT_MS;  
	
	int optval;

#ifdef _WIN32
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&wtimeout,sizeof (int)) < 0)
		perror("setsockopt failed\n");	
#else
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0)
		perror("setsockopt failed\n");
		
	optval = 7; // high priority.
	if (setsockopt(fd, SOL_SOCKET, SO_PRIORITY, &optval, sizeof(optval))<0) {
        perror("UDP socket: SO_PRIORITY");
    }
#endif

	optval=65535; 
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char *)&optval, sizeof(optval))<0) {
      perror("data_socket: SO_SNDBUF");
    }
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char *)&optval, sizeof(optval))<0) {
      perror("data_socket: SO_RCVBUF");
    }

	/* bind the socket to any valid IP address and a specific port */
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(SERVICE_PORT);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}
	
	if ((sock_TCP_Server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP )) < 0)
	{
		perror("socket tcp");
		return -1;
	}
	int tcpmaxseg = 1032;
	int yes = 1;
	setsockopt(sock_TCP_Server, IPPROTO_TCP, TCP_MAXSEG, (const char *)&tcpmaxseg, sizeof(int));
	int sndbufsize = 65535;
	int rcvbufsize = 65535;
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_SNDBUF, (const char *)&sndbufsize, sizeof(int));
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVBUF, (const char *)&rcvbufsize, sizeof(int));
#ifdef _WIN32
	if (setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVTIMEO, (const char *)&wtimeout, sizeof(wtimeout)) < 0)
		perror("setsockopt failed\n");
#else
	if (setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0)
		perror("setsockopt failed\n");		
#endif
	setsockopt(sock_TCP_Server, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
	
#ifndef _WIN32
	if (setsockopt(sock_TCP_Server, SOL_SOCKET, SO_PRIORITY, &optval, sizeof(optval))<0) {
        perror("sock_TCP_Server socket: SO_PRIORITY");
    }
#endif
	
	if (bind(sock_TCP_Server, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
	{
		perror("bind tcp");
		return -1;
	}
	listen(sock_TCP_Server, 1024);
	
#ifdef _WIN32
	int addrlen;
	unsigned long mode;
#else
	int fl;
	socklen_t addrlen;
#endif
	
#ifdef _WIN32
	mode = 1;
	ioctlsocket(sock_TCP_Server, FIONBIO, &mode);
#else
	int flags = fcntl(sock_TCP_Server, F_GETFL, 0);
    fcntl(sock_TCP_Server, F_SETFL, flags | O_NONBLOCK);
#endif

	start_rb_register_thread();
	
	start_temperature_thread();
	
	start_rb_pa_thread();
	
	signal(SIGINT, handle_sigint);
	
	runRadioberry();
	
	closeRadioberry();
	
	return 0;
}

int closeRadioberry() {
	fprintf(stderr, "close radioberry\n");
	
	deinit_stream();
	
	if (fd_rb != 0) close(fd_rb);
	if (sock_TCP_Client >= 0) close(sock_TCP_Client);
	if (sock_TCP_Server >= 0) close(sock_TCP_Server);
#ifdef _WIN32
	closesocket(fd);
	WSACleanup();
#endif

	return 0;
}
//end of source.