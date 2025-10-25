#ifndef __RADIOBERRY_H__
#define __RADIOBERRY_H__


#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <netinet/tcp.h>


#define FIRMWAREVERSION "2025.10.25"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

void printIntroScreen() {
	fprintf(stderr,"\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr, "\t\t\tRadioberry V2.0\n\n\n");
	fprintf(stderr,	"\tSupports 4 receivers and 1 transmitter.\n\n");
	fprintf(stderr, "\tBuild version: %s\n\n", FIRMWAREVERSION);
	fprintf(stderr, "\tHave fun Johan PA3GSB\n\n");
	fprintf(stderr, "\tReport requests or bugs to <pa3gsb@gmail.com>.\n");
	fprintf(stderr, "====================================================================\n");
	fprintf(stderr, "====================================================================\n");
}

int sys_temp = 0; //rpi-temperature.

int pa_temp = 0;
int pa_current = 0;
int pa_temp_ok = 1;

//ringbuffer for handling SPI commands.
#define CAPACITY 64
uint32_t spi_commands[CAPACITY] = {0};
uint32_t p_read = 0;
uint32_t p_write = 0;
static inline int size(void)     { return p_write - p_read; }
static inline int empty(void)    { return p_read == p_write; }
static inline int full(void)     { return size() == CAPACITY; }
static inline int mask(int val)  { return val & (CAPACITY - 1); }
static inline int push(int val)  { assert(!full()); spi_commands[mask(p_write++)] = val; }
static inline int pop(void)      { assert(!empty()); return spi_commands[mask(p_read++)]; }

char rb_control = 0x00;

int fd_rb;
char rx_buffer[512];
int rb_sample = 0;

unsigned char command = 0;
uint32_t command_data = 0;
uint32_t commands[256];
unsigned char run = 0;
int closerb = 0;

int rb_sleep = 100;

static int initRadioberry(void);
static void runRadioberry(void);
static void closeRadioberry(void);

static void sendPacket(void);
static void handlePacket(char* buffer);
static void processPacket(char* buffer);
static void fillPacketToSend(void);

static void *packetreader(void *arg);
static void *txWriter(void *arg);

static void send_control(unsigned char command);
static float timedifference_msec(struct timeval t0, struct timeval t1);

static void put_tx_buffer(unsigned char  value);
static unsigned char get_tx_buffer(void);

int sock_TCP_Server = -1;
int sock_TCP_Client = -1;

int udp_retries=0;
int active = 0;

static volatile int keepRunning = 1;

#define TX_MAX 4800
#define TX_MAX_BUFFER (TX_MAX * 8)
unsigned char tx_buffer[TX_MAX_BUFFER];
int fill_tx = 0; 
int use_tx  = 0;

int gateware_major_version = 0;
int gateware_minor_version = 0;
int gateware_fpga_type = 0;
int driver_version = 0.0;

char CWX = 0;
char MOX = 0;
int save_mox = -1;
sem_t tx_empty;
sem_t tx_full;
sem_t mutex;
sem_t spi_msg;
sem_t i2c_meas;

int tx_count =0;
void rx_reader(unsigned char iqdata[]);

unsigned char iqdata[6];
unsigned char tx_iqdata[4];

#define SERVICE_PORT	1024

int nrx = 1; // n Receivers
int lnrx = 1;

#define SYNC 0x7F
uint32_t last_sequence_number = 0;
uint32_t last_seqnum=0xffffffff, seqnum; 
		
unsigned char hpsdrdata[1032];
uint8_t header_hpsdrdata[4] = { 0xef, 0xfe, 1, 6 };
uint8_t sync_hpsdrdata[8] = { SYNC, SYNC, SYNC, 0, 0, 0, 0, 0};

unsigned char broadcastReply[60];
#define TIMEOUT_MS      100     

int running = 0;
int fd;									/* our socket */

struct sockaddr_in myaddr;				/* our address */
struct sockaddr_in remaddr;				/* remote address */

socklen_t addrlen = sizeof(remaddr);	/* length of addresses */

struct timeval t20;
struct timeval t21;
float elapsed;


float timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

void handle_sigint(int sig) 
{ 
	if (running) fprintf(stderr, "  SDR program is still running; please stop SDR first.\n");
	closerb = 1;
}
#endif
