int initRadioberry();
void runRadioberry(void);
void closeRadioberry();

void sendPacket(void);
void handlePacket(char* buffer);
void processPacket(char* buffer);
void fillDiscoveryReplyMessage(void);
void fillPacketToSend(void);
void printIntroScreen(void);
void *packetreader(void *arg);
void *spiWriter(void *arg);
void *spiReader(void *arg);
void spi_control_tx();
void spi_send_control();
float timedifference_msec(struct timeval t0, struct timeval t1);
void initVSWR();

void put_tx_buffer(unsigned char  value);
unsigned char get_tx_buffer(void);

int sock_TCP_Server = -1;
int sock_TCP_Client = -1;
static volatile int keepRunning = 1;

#define TX_MAX 4800
#define TX_MAX_BUFFER (TX_MAX * 8)
unsigned char tx_buffer[TX_MAX_BUFFER];
int fill_tx = 0; 
int use_tx  = 0;
unsigned char drive_level;
unsigned char prev_drive_level = 0;
int MOX = 0;
int saveMox = -1;
sem_t tx_empty;
sem_t tx_full;
sem_t mutex;

#define SERVICE_PORT	1024
#define SYNC 0x7F
uint32_t last_sequence_number = 0;
uint32_t last_seqnum=0xffffffff, seqnum; 

#define FIRMWARE_VERSION 0x29		//v4.1 firmware version
unsigned char hpsdrdata[1032];
uint8_t header_hpsdrdata[4] = { 0xef, 0xfe, 1, 6 };
uint8_t sync_hpsdrdata[8] = { SYNC, SYNC, SYNC, 0, 0, 0, 0, FIRMWARE_VERSION};

unsigned char broadcastReply[60];
#define TIMEOUT_MS      1500     

int running = 0;
int fd;									/* our socket */

struct sockaddr_in myaddr;				/* our address */
struct sockaddr_in remaddr;				/* remote address */

socklen_t addrlen = sizeof(remaddr);	/* length of addresses */
int recvlen;							/* # bytes received */

struct timeval t20;
struct timeval t21;
float elapsed;

#define MAX11613_ADDRESS	0x34
unsigned char data[8];
unsigned int i2c_bus = 1;
int i2c_handler = 0;
int vswr_active = 0;

int att11 = 0;
int prevatt11 = 0;
int att523 = 0;
int prevatt523 = 0;
int change = 0x01;
int pwmin = 0; int prev_pwim = 0;
int pwmax = 1023; int prev_pwmax = 1023;


int udp_retries=0;
int active = 0;

#define RX_MAX 10
int tx_count =0;

sem_t rx_empty;
sem_t rx_full;
sem_t tx_empty;
sem_t tx_full;
sem_t mutex;

unsigned char rx_buffer[RX_MAX][504];
int rx_read_index = 0;

void rx1_spiReader(unsigned char iqdata[]);
void rx2_spiReader(unsigned char iqdata[]);

static int rx1_spi_handler;
static int rx2_spi_handler;

unsigned char iqdata[6 * 64];
unsigned char tx_iqdata[4];

int nrx = 1; // n Receivers

int freq = 395763341; //phase
int freq2 = 395763341;
int txfreq = 395763341;

int att = 0;
int dither = 0;
int rando = 0;
int sampleSpeed = 0;

#define ADDR_ALEX 			0x21 		/* PCA9555 address 1 */
int i2c_alex_handler = 0;
int i2c_alex = 0;
int alex_manual = 0;
uint16_t i2c_alex_data = 0;
uint16_t i2c_data = 0;

#define RX1_SAMPLES_PIN 33       
#define PTT_PIN 40      

int rx1_phase = 0;
int rx2_phase = 0;
int tx_phase = 0;
	

