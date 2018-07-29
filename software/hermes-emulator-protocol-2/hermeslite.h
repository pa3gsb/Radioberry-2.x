#define MAX_RECEIVERS 2
#define SERVICE_PORT 1024
#define MAX_DATA_SIZE 2048
#define HERMESLITE  0x06	
#define FIRMWARE_VERSION 0x28	//firmware version 4.0 (=0x28)
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


extern int use_local_audio_out;
extern int use_local_audio_in;

void print_version(void);
void usage(void);
void printIntroScreen();
int handleDiscovery(unsigned char* buffer);
int createUDPSocket(void);
void handle_packets_from_sdr_program(unsigned char* buffer,int buflen);
void *send_rx_iq_to_host(void *arg);
void *send_high_priority_status_to_host(void *arg);
void rx1_spiReader(unsigned char iqdata[]);
void rx2_spiReader(unsigned char iqdata[]);
void receiver_specific_registers_from_host_port(unsigned char* buffer);
void transmitter_specific_registers_from_host_port(unsigned char* buffer);
void high_priority_from_host_port(unsigned char* buffer);
void tx_iq_from_host_port(unsigned char* buffer);
int initialize_gpio();
int handle_data_from_sdr_program();
void create_radioberry_socket();
void process_local_mic(unsigned char *mic_buffer);