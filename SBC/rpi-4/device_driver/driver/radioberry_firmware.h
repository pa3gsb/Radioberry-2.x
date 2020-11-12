#ifndef __RADIOBERRY_FIRMWARE_H__
#define __RADIOBERRY_FIRMWARE_H__

#include <linux/delay.h>

#define PI_ALT0   4

#define RPI_RX_CLK 6
#define RPI_TX_CLK 4

#define RPI_SPI_CE0   8
#define RPI_SPI_CE1   7
#define RPI_SPI_SCLK 11
#define RPI_SPI_MISO  9
#define RPI_SPI_MOSI 10

#define PI_SPI_FLAGS_CHANNEL(x)    ((x&7)<<29)
#define PI_SPI_FLAGS_GET_CHANNEL(x) (((x)>>29)&7)
#define PI_SPI_FLAGS_GET_CSPOLS(x)  (((x)>>2)&7)
#define PI_SPI_FLAGS_GET_MODE(x)     ((x)&3)

#define SPI_CS_CSPOLS(x) ((x)<<21)
#define SPI_CS_TXD         (1<<18)
#define SPI_CS_RXD         (1<<17)
#define SPI_CS_DONE        (1<<16)
#define SPI_CS_REN         (1<<12)
#define SPI_CS_TA          (1<<7)
#define SPI_CS_CSPOL(x)  ((x)<<6)
#define SPI_CS_CLEAR(x)  ((x)<<4)
#define SPI_CS_MODE(x)   ((x)<<2)
#define SPI_CS_CS(x)     ((x)<<0)

#define SPI_CS   0
#define SPI_FIFO 1
#define SPI_CLK  2
#define SPI_DLEN 3
#define SPI_LTOH 4
#define SPI_DC   5

static int initialize_firmware(void);
int rxStream(int nrx, unsigned char stream[]);
void read_iq_sample(int lnrx, int iqs, unsigned char iqdata[]);
int spiXfer(char *txBuf, char *rxBuf, unsigned cnt);
void write_iq_sample(unsigned char tx_iqdata[]);

static int initialize_firmware() {	

	//SPI mode pins
	setPinMode(RPI_SPI_CE0,  PI_ALT0);
	setPinMode(RPI_SPI_CE1,  PI_ALT0);

	setPinMode(RPI_SPI_SCLK, PI_ALT0);
	setPinMode(RPI_SPI_MISO, PI_ALT0);
	setPinMode(RPI_SPI_MOSI, PI_ALT0);

	//RX IO Init part 
	initialize_gpio_for_output(rpi_io, RPI_RX_CLK);
	*rpi_set_io_low = (1<<RPI_RX_CLK); 	// init pi-rx_clk
	
	initialize_gpio_for_input(rpi_io, 21);	// rx iq data
	initialize_gpio_for_input(rpi_io, 20);	// rx iq data
	initialize_gpio_for_input(rpi_io, 19);	// rx iq data
	initialize_gpio_for_input(rpi_io, 16);	// rx iq data
	initialize_gpio_for_input(rpi_io, 25);	// available samples.
	
	//TX IO Init part 
	initialize_gpio_for_output(rpi_io, RPI_TX_CLK);
	*rpi_set_io_low = (1<<RPI_TX_CLK); 	// init pi-tx_clk
	initialize_gpio_for_output(rpi_io, 5);	// tx iq data
	initialize_gpio_for_output(rpi_io, 12);	// tx iq data
	initialize_gpio_for_output(rpi_io, 17);	// tx iq data
	initialize_gpio_for_output(rpi_io, 18);	// tx iq data
	
	printk(KERN_INFO "GPIO ready for rx and tx streaming...\n");
}

int spiXfer(char *txBuf, char *rxBuf, unsigned cnt){

	unsigned speed = 9000000; 
	uint32_t flags = 49155;

	uint32_t spi_dummy;
	unsigned txCnt=0;
	unsigned rxCnt=0;
	uint32_t spiDefaults;
	unsigned mode, channel, cspol, cspols;
   
   flags = flags | PI_SPI_FLAGS_CHANNEL(0); // SPI0

   channel = PI_SPI_FLAGS_GET_CHANNEL(flags);
   mode   =  PI_SPI_FLAGS_GET_MODE   (flags);
   cspols =  PI_SPI_FLAGS_GET_CSPOLS(flags);
   cspol  =  (cspols>>channel) & 1;

   spiDefaults = SPI_CS_MODE(mode)     |
                 SPI_CS_CSPOLS(cspols) |
                 SPI_CS_CS(channel)    |
                 SPI_CS_CSPOL(cspol)   |
                 SPI_CS_CLEAR(3);

   spiReg[SPI_DLEN] = 2; /* undocumented, stops inter-byte gap */

   spiReg[SPI_CS] = spiDefaults; /* stop */

   spiReg[SPI_CLK] = 250000000/speed;

   spiReg[SPI_CS] = spiDefaults | SPI_CS_TA; /* start */

   while((txCnt < cnt) || (rxCnt < cnt))
   {
      while((rxCnt < cnt) && ((spiReg[SPI_CS] & SPI_CS_RXD)))
      {
         if (rxBuf) rxBuf[rxCnt] = spiReg[SPI_FIFO];
         else       spi_dummy    = spiReg[SPI_FIFO];
         rxCnt++;
      }

      while((txCnt < cnt) && ((spiReg[SPI_CS] & SPI_CS_TXD)))
      {
         if (txBuf) spiReg[SPI_FIFO] = txBuf[txCnt];
         else       spiReg[SPI_FIFO] = 0;
         txCnt++;
      }
   }

   while (!(spiReg[SPI_CS] & SPI_CS_DONE)) ;

   spiReg[SPI_CS] = spiDefaults; /* stop */
}

int rxStream(int nrx, unsigned char stream[]){
	unsigned char iqdata[6];
	int iqs = 1;

	// using irq in driver
	// no need to poll... calling 76 samples and place in circulair buffer.
	//while ((((*rpi_read_io) >> 25) & 1) == 0) {return 0;}//wait for enough samples 
	
	int nr_samples = (nrx == 1)? 63 : (nrx == 2)? 72: (nrx ==3)? 75: 76;
	
	int s = 0;
	for (s = 0; s < nr_samples; s++) {	
		read_iq_sample(nrx, iqs, iqdata);
		memcpy(stream + (s * 6), iqdata, 6);
		iqs++;
	}
	return nr_samples * 6;
}

void read_iq_sample(int lnrx, int iqs, unsigned char iqdata[]){
	uint32_t value = 0;
	
	int i = 0;	
	for (i = 0; i < 6 ; i++) {
		
		*rpi_set_io_high = (1<<RPI_RX_CLK);
		value = *rpi_read_io;

		iqdata[i] =  (((value >> 16) & 1) << 7);
		iqdata[i] |=  (((value >> 19) & 1) << 6);
		iqdata[i] |=  (((value >> 20) & 1) << 5);
		iqdata[i] |=  (((value >> 21) & 1) << 4);
		
		*rpi_set_io_low = (1<<RPI_RX_CLK);
		value = *rpi_read_io;
	
		iqdata[i] |=  (((value >> 16) & 1) << 3);
		iqdata[i] |=  (((value >> 19) & 1) << 2);
		iqdata[i] |=  (((value >> 20) & 1) << 1);
		iqdata[i] |=  (((value >> 21) & 1));			
	}		
}


void write_iq_sample(unsigned char tx_iqdata[]){

	int i = 0;
	for (i = 0; i < 4 ; i++) {
		if (tx_iqdata[i] & 0x80) *rpi_set_io_high = (1<<17); else *rpi_set_io_low = (1<<17);
		if (tx_iqdata[i] & 0x40) *rpi_set_io_high = (1<<5);  else *rpi_set_io_low = (1<<5);
		if (tx_iqdata[i] & 0x20) *rpi_set_io_high = (1<<18); else *rpi_set_io_low = (1<<18);
		if (tx_iqdata[i] & 0x10) *rpi_set_io_high = (1<<12); else *rpi_set_io_low = (1<<12);
		*rpi_set_io_high = (1<<RPI_TX_CLK);
		
		if (tx_iqdata[i] & 0x08) *rpi_set_io_high = (1<<17); else *rpi_set_io_low = (1<<17);
		if (tx_iqdata[i] & 0x04) *rpi_set_io_high = (1<<5);  else *rpi_set_io_low = (1<<5);
		if (tx_iqdata[i] & 0x02) *rpi_set_io_high = (1<<18); else *rpi_set_io_low = (1<<18);
		if (tx_iqdata[i] & 0x01) *rpi_set_io_high = (1<<12); else *rpi_set_io_low = (1<<12);
		*rpi_set_io_low = (1<<RPI_TX_CLK);
	}
}

#endif

