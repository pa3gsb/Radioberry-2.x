#ifndef __RADIOBERRY_FIRMWARE_H__
#define __RADIOBERRY_FIRMWARE_H__

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

#define SAMPLE_BYTES 512
static int initialize_firmware(void);
int spiXfer(int spi_channel, char *txBuf, char *rxBuf, unsigned cnt);
int write_iq_sample(unsigned char tx_iqdata[]);

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
	
	initialize_gpio_for_input(rpi_io, 23);	// rx iq data
	initialize_gpio_for_input(rpi_io, 20);	// rx iq data
	initialize_gpio_for_input(rpi_io, 19);	// rx iq data
	initialize_gpio_for_input(rpi_io, 18);	// rx iq data
	initialize_gpio_for_input(rpi_io, 16);	// rx iq data
	initialize_gpio_for_input(rpi_io, 13);	// rx iq data
	initialize_gpio_for_input(rpi_io, 12);	// rx iq data
	initialize_gpio_for_input(rpi_io,  5);	// rx iq data
	initialize_gpio_for_input(rpi_io, 25);	// available samples.
	
	printk(KERN_INFO "GPIO ready for rx and tx streaming...\n");
	
	return 0;
}

int spiXfer(int spi_channel, char *txBuf, char *rxBuf, unsigned cnt){

	unsigned speed = 9000000; 
	uint32_t flags = 49155;

	uint32_t spi_dummy;
	unsigned txCnt=0;
	unsigned rxCnt=0;
	uint32_t spiDefaults;
	unsigned mode, channel, cspol, cspols;
   
   flags = flags | PI_SPI_FLAGS_CHANNEL(spi_channel); 

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
   
   return 0;
}

int write_iq_sample(unsigned char tx_iqdata[]) {

	unsigned char data[4];
	
	mutex_lock(&spi_mutex);
	spiXfer(1, tx_iqdata, data, 4); //spi channel 1; write IQ sample (4bytes)
	mutex_unlock(&spi_mutex);
	
	return 4;
}

#endif

