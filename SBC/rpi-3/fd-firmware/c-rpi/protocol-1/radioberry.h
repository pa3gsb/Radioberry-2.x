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
void spi_control_rx1_phase();
void spi_control_rx2_phase();
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

int hold_nrx=0;
int nrx = 2; // n Receivers
int holdfreq = 0;
int holdfreq2 = 0;
int holdtxfreq = 0;
int freq = 4706000;
int freq2 = 1008000;
int txfreq = 3630000;

int rx1_phase = 0;
int rx2_phase = 0;
int tx_phase = 0;

int att = 0;
int holdatt =128;
int holddither=128;
int dither = 0;
int rando = 0;
int sampleSpeed = 0;
int holdsampleSpeed = 0;

int pureSignal = 0;
int holdPureSignal = 128;

int vna = 0;
int holdvna = 1;
int vna_att_20dB = 0;
int holdvna_att_20dB = 1;

#define SYNC 0x7F
uint32_t last_sequence_number = 0;
uint32_t last_seqnum=0xffffffff, seqnum; 

#define FIRMWARE_VERSION 0x29		//v4.1 firmware version
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