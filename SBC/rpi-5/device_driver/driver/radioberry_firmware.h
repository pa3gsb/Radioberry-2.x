#ifndef __RADIOBERRY_FIRMWARE_H__
#define __RADIOBERRY_FIRMWARE_H__

#define GPIO_FUNC_SPI 0  
#define RPI_RX_CLK 6

#define RPI_SPI_CE0   8
#define RPI_SPI_CE1   7
#define RPI_SPI_SCLK 11
#define RPI_SPI_MISO  9
#define RPI_SPI_MOSI 10

static struct spi_device *spi_ctrl_dev;
static struct spi_device *spi_tx_dev;

static int initialize_firmware(void);
int rxStream(int nrx, unsigned char stream[]);
void read_iq_sample(int lnrx, int iqs, unsigned char iqdata[]);
int spiXfer(int spi_channel, char *txBuf, char *rxBuf, unsigned cnt);
int write_iq_sample(unsigned char tx_iqdata[]);

static int initialize_firmware() {	

	printk(KERN_INFO "initialize_firmware: make GPIO ready for rx and tx streaming...\n");
	
	// disable the SPI
    *(volatile uint32_t *)(SPIBase + DW_SPI_SSIENR) = 0x0;
	
	// SPI Mode Pins
	setPinMode(RPI_SPI_CE0,  GPIO_FUNC_SPI);
	setPinMode(RPI_SPI_CE1,  GPIO_FUNC_SPI);

	setPinMode(RPI_SPI_SCLK, GPIO_FUNC_SPI);
	setPinMode(RPI_SPI_MISO, GPIO_FUNC_SPI);
	setPinMode(RPI_SPI_MOSI, GPIO_FUNC_SPI);
	
	// SPI control settings
	// set the speed - this is the divisor from 200MHz in the RPi5
    *(volatile uint32_t *)(SPIBase + DW_SPI_BAUDR) = 20;  
    // set mode - CPOL = 1, CPHA = 1 (Mode 3)
	*(volatile uint32_t *)(SPIBase + DW_SPI_CTRLR0) |= DW_PSSI_CTRLR0_SCPHA;
	*(volatile uint32_t *)(SPIBase + DW_SPI_CTRLR0) |= DW_PSSI_CTRLR0_SCPOL;	

	// enable the SPI
    *(volatile uint32_t *)(SPIBase + DW_SPI_SSIENR) = 0x1;

	//RX IO Init part 
	initialize_gpio_for_output(RPI_RX_CLK);
	clr_pin(RPI_RX_CLK); 			// init pi-rx_clk
	
	initialize_gpio_for_input(23);	// rx iq data
	initialize_gpio_for_input(20);	// rx iq data
	initialize_gpio_for_input(19);	// rx iq data
	initialize_gpio_for_input(18);	// rx iq data
	initialize_gpio_for_input(16);	// rx iq data
	initialize_gpio_for_input(13);	// rx iq data
	initialize_gpio_for_input(12);	// rx iq data
	initialize_gpio_for_input( 5);	// rx iq data
	
	initialize_gpio_for_input(25);	// available samples.
	
	printk(KERN_INFO "GPIO ready for rx and tx streaming...\n");
	
	return 0;
}


int spi_ctrl_Xfer(int spi_channel, char *txBuf, char *rxBuf, unsigned cnt){
	
	//printk(KERN_INFO "inside %s function \n", __FUNCTION__);
	
	struct spi_transfer t = {
		.tx_buf = txBuf,
		.rx_buf = rxBuf,
		.len = cnt,
	};
	struct spi_message m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	
	//spi_ctrl_dev->chip_select = spi_channel;

	// Send and receive the message to/from the Radioberry SPI device
	int ret = spi_sync(spi_ctrl_dev, &m);
	if (ret) {
		pr_err("SPI transfer failed\n");
		return ret;
	} 
	
   return 0;
}

/*
Using the spi driver was not working fast enough. 
int spi_tx_Xfer(int spi_channel, char *txBuf, char *rxBuf, unsigned cnt){
	
	//printk(KERN_INFO "inside %s function \n", __FUNCTION__);
	
	struct spi_transfer t = {
		.tx_buf = txBuf,
		.rx_buf = rxBuf,
		.len = cnt,
	};
	struct spi_message m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	
	// Send and receive the message to/from the Radioberry SPI device
	int ret = spi_sync(spi_tx_dev, &m);
	if (ret) {
		pr_err("SPI transfer failed\n");
		return ret;
	} 
	
   return 0;
}
*/


int spi_tx_Xfer(int spi_channel, char *txBuf, char *rxBuf, unsigned cnt)
{
	//printk(KERN_INFO "inside %s function \n", __FUNCTION__);
	
	// set the frame size to 32 bits
    *(volatile uint32_t *)(SPIBase + DW_SPI_CTRLR0) = ((*(volatile uint32_t *)(SPIBase + DW_SPI_CTRLR0)) | DW_PSSI_CTRLR0_DFS32_MASK | DW_PSSI_CTRLR0_DFS_MASK);

	// write the tx fifo
	unsigned txCnt = 0;
    while((*(volatile uint32_t *)(SPIBase + DW_SPI_SR) & DW_SPI_SR_TF_NOT_FULL) && (txCnt < cnt))
    {
        *(volatile uint8_t *)(SPIBase + DW_SPI_DR) = (uint8_t)txBuf[txCnt];
        txCnt++;
    }
		//spi control
    *(volatile uint32_t *)(SPIBase + DW_SPI_SER) = 1 << spi_channel;  // CE1  active

    // transfer the tx fifo and fill the rx fifo   
    int rxCnt = 0;
    while(rxCnt < cnt)
    {
        // check if there is data to read (check status register for Read Fifo Not Empty)
        if(*(volatile uint32_t *)(SPIBase + DW_SPI_SR) & DW_SPI_SR_RF_NOT_EMPT)
        {
            rxBuf[rxCnt] = *(volatile uint8_t *)(SPIBase + DW_SPI_DR);
            rxCnt++;
        } 
    }
	
	//spi control
    *(volatile uint32_t *)(SPIBase + DW_SPI_SER) = 0x00;	//CE1 inactive

    return 0;
}

int rxStream(int nrx, unsigned char stream[]){
	unsigned char iqdata[6];
	int iqs = 1;
	
	int nr_samples = (nrx == 1)? 63 : (nrx == 2)? 72: (nrx ==3)? 75: (nrx ==4)? 76: (nrx ==5)? 75: (nrx ==6)? 78: (nrx ==7)? 77: 80;
	
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
			
	for (int i = 0; i < 6 ; i++) {
		
		if (i % 2 == 0) set_pin(RPI_RX_CLK); else clr_pin(RPI_RX_CLK);
		ndelay(100);
		value = read_pin_all();
		
		iqdata[i] =   (((value >> 23) & 1) << 7);
		iqdata[i] |=  (((value >> 20) & 1) << 6);
		iqdata[i] |=  (((value >> 19) & 1) << 5);
		iqdata[i] |=  (((value >> 18) & 1) << 4);
		iqdata[i] |=  (((value >> 16) & 1) << 3);
		iqdata[i] |=  (((value >> 13) & 1) << 2);
		iqdata[i] |=  (((value >> 12) & 1) << 1);
		iqdata[i] |=  (((value >> 5) & 1));						
	}		
}

int write_iq_sample(unsigned char tx_iqdata[]) {

	unsigned char data[4];
	
	mutex_lock(&spi_mutex);
	spi_tx_Xfer(1, tx_iqdata, data, 4); //spi channel 1; write IQ sample (4bytes)
	mutex_unlock(&spi_mutex);
	
	return 4;
}

#endif

