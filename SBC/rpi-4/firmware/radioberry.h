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

#include <pigpio.h>

#define RPI_RX_CLK 6
#define RPI_TX_CLK 4


void printIntroScreen() {
	fprintf(stderr,"\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr, "\tRadioberry V2.0\n");
	fprintf(stderr,	"\n");
	fprintf(stderr,	"*** EXPERIMENTAL VERSION USING LOW LEVEL CALLS TO GPIO ****\n");
	fprintf(stderr,	"*** !!! This version runs at RPI-4 only!!!! ****\n");
	fprintf(stderr,	"\n\n");
	fprintf(stderr,	"Supports max 4 receivers and 1 transmitter. \n");
	fprintf(stderr,	"\n\n");
	fprintf(stderr, "\t\t\t Have fune Johan PA3GSB\n");
	fprintf(stderr, "\n\n");
	fprintf(stderr, "\n\tReport bugs to <pa3gsb@gmail.com>.\n");
	fprintf(stderr, "====================================================================\n");
	fprintf(stderr, "====================================================================\n");
}

unsigned char command = 0;
uint32_t command_data = 0;
uint32_t commands[256];
unsigned char run = 0;

int rb_sleep = 0;
int iqs =0 ;		//iq-sample count

int initRadioberry();
void runRadioberry(void);
void closeRadioberry();

void sendPacket(void);
void handlePacket(char* buffer);
void processPacket(char* buffer);
//void fillDiscoveryReplyMessage(void);
void fillPacketToSend(void);

void *packetreader(void *arg);
void *spiWriter(void *arg);
//void spi_control_rx1_phase();
//void spi_control_rx2_phase();
//void spi_control_tx();
void spi_send_control(unsigned char command);
float timedifference_msec(struct timeval t0, struct timeval t1);
//void initVSWR();

void put_tx_buffer(unsigned char  value);
unsigned char get_tx_buffer(void);

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

int MOX = 0;
sem_t tx_empty;
sem_t tx_full;
sem_t mutex;

int tx_count =0;
void rx_reader(unsigned char iqdata[]);
void rx2_reader(unsigned char iqdata[]);

static int rx1_spi_handler;
static int rx2_spi_handler;

unsigned char iqdata[6];
unsigned char tx_iqdata[8];

#define SERVICE_PORT	1024


int nrx = 1; // n Receivers



#define SYNC 0x7F
uint32_t last_sequence_number = 0;
uint32_t last_seqnum=0xffffffff, seqnum; 

#define NR 0x04
#define HERMESLITE  0x06
#define FIRMWARE_VERSION 0x44		//68P4
#define MINOR_VERSION 0x04		
unsigned char hpsdrdata[1032];
uint8_t header_hpsdrdata[4] = { 0xef, 0xfe, 1, 6 };
uint8_t sync_hpsdrdata[8] = { SYNC, SYNC, SYNC, 0, 0, 0, 0, FIRMWARE_VERSION};

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