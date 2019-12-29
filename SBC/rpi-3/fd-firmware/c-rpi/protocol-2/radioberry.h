#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <semaphore.h>
#include <math.h>
#include <malloc.h>
#include <signal.h>
#include <stdbool.h>
#include <linux/if_packet.h>
#include <netinet/in.h>		 
#include <netinet/if_ether.h>  
#include <netinet/ip.h>	
#include <netinet/udp.h>		
#include <netinet/tcp.h>
#include <sys/time.h>
#include "udp.h"
#include <pigpio.h>
#include <getopt.h>

#define NR_ADC 1
#define MAX_RECEIVERS 2
#define SERVICE_PORT 1024
#define MAX_DATA_SIZE 2048
//ethernet protocol 3.7 (protocol - 2): 
//0: board = "ATLAS"
//1: board = “HERMES” (ANAN-10,100)
//2: board = “HERMES” (ANAN-10E, 100B)
//3: board = “ANGELA” (ANAN-100D)
//4: board = “ORION” (ANAN-200D)
//5: board = “ORION Mk II” (ANAN-8000DLE)
//6: board = “Hermes Lite”						Not found by SDR Console
// In the discovery reply some board info can be set according to protocol
// seems not always be handled accordingly.
#define HERMESLITE  0x02 //0x02	
#define FIRMWARE_VERSION 103	//thetis 2.6.7 needs this version!
#define TIMEOUT_MS 100 
#define HANDLER_STEADY_TIME_US 5000

// port definitions from host
#define GENERAL_REGISTERS_FROM_HOST_PORT 1024
#define RECEIVER_SPECIFIC_REGISTERS_FROM_HOST_PORT 1025
#define TRANSMITTER_SPECIFIC_REGISTERS_FROM_HOST_PORT 1026
#define HIGH_PRIORITY_FROM_HOST_PORT 1027
#define AUDIO_FROM_HOST_PORT 1028
#define TX_IQ_FROM_HOST_PORT 1029

// port definitions to host
#define COMMAND_RESPONCE_TO_HOST_PORT 1024
#define HIGH_PRIORITY_TO_HOST_PORT 1025
#define MIC_LINE_TO_HOST_PORT 1026
#define WIDE_BAND_TO_HOST_PORT 1027
#define RX_IQ_TO_HOST_PORT_0 1035

//extern int use_local_audio_out;
//extern int use_local_audio_in;

void spi_control_rx1_phase();
void spi_control_rx2_phase();
void spi_control_tx();
void print_version(void);
void usage(void);
void printIntroScreen();
int handleDiscovery(unsigned char* buffer);
int createUDPSocket(void);
void handle_packets_from_sdr_program(unsigned char* buffer,int buflen);
//void *send_rx_iq_to_host(void *arg);
void send_rx_iq_to_host();
void *send_high_priority_status_to_host(void *arg);
void spi_rx1_writer();
void spi_rx2_writer();
void spi_tx_writer();
void rx1_reader(unsigned char iqdata[]);
void rx2_reader(unsigned char iqdata[]);
void receiver_specific_registers_from_host_port(unsigned char* buffer);
void transmitter_specific_registers_from_host_port(unsigned char* buffer);
void high_priority_from_host_port(unsigned char* buffer);
void tx_iq_from_host_port(unsigned char* buffer);
int initialize_gpio();
//int handle_data_from_sdr_program();
void *handle_data_from_sdr_program(void *arg);
void create_radioberry_socket();
void process_local_mic(unsigned char *mic_buffer);
void start_dummy_mic_thread();
void *mic_dummy_thread(void *arg);

static long rx1_sequence = 0;
static long rx2_sequence = 0;
static char sdr_client_addr[20];
static char radioberry_addr[20];
static long status_sequence = 0;
static long mic_sequence = 0;
static long mic_count=4;

static struct sockaddr_in src_addr[8];
static struct sockaddr_in dst_addr;

static struct sockaddr_in high_priority_addr;
static int high_priority_addr_length;

static struct sockaddr_in mic_line_addr;
static int mic_line_addr_length; 

static int radioberry_socket=-1;
static int discover_socket=-1;
static int remote_port =0;

unsigned char broadcastReply[60];

static int rx1_spi_handler;
static int rx2_spi_handler;

//static int use_local_audio_in;
//static int use_local_audio_out;

static int running = 0;
static int nrx = 1; 
static int sampleSpeed[MAX_RECEIVERS];
static int samplespeed;
static int ddc[MAX_RECEIVERS];
static int ptt = 0;
static int gain = 0;
static int dither = 1;

static int rxfreq1 = 4706000;
static int rx1_phase = 0;
static int rxfreq2 = 1008000;
static int rx2_phase = 0;
static int txfreq  = 3630000;
static int tx_phase = 0;

static unsigned char drive_level;
static unsigned char prev_drive_level = -1;

static sem_t mutex;
static sem_t mic_sample;
static sem_t high_priority_mutex;

struct timeval t1;
struct timeval t2;
struct timeval t1a;
struct timeval t2a;

static int lcount=0;
static int lcounta=0;
static int lseq=-1;

static int cw = 0;
static int cw_keyer_speed = 0;
static int cw_keyer_weight = 0;
static int cw_iambic_mode = 0;
static int cw_keyer_reverse = 0;
static int cw_break_in = 0;
static int cw_ptt = 0;
