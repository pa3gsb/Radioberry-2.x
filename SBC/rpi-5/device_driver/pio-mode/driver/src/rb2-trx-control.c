
#include <linux/printk.h>

#include "rb2-trx-control.h"
#include "rb2-rpi5.h"

struct spi_device *spi_ctrl_dev = NULL;


int rb2_trx_initialize() {	

	printk(KERN_INFO "initialize_firmware: make GPIO ready for rx and tx streaming...\n");
	
	// Radioberry control using SPI Mode Pins
	setPinMode(RPI_SPI_CE0,  GPIO_FUNC_SPI);
	setPinMode(RPI_SPI_CE1,  GPIO_FUNC_SPI);

	setPinMode(RPI_SPI_SCLK, GPIO_FUNC_SPI);
	setPinMode(RPI_SPI_MISO, GPIO_FUNC_SPI);
	setPinMode(RPI_SPI_MOSI, GPIO_FUNC_SPI);
	

	//RX IO init.
	initialize_gpio_for_output(RPI_RX_CLK);
	clr_pin(RPI_RX_CLK); 			// init pi-rx_clk
	initialize_gpio_for_input(25);	// available samples.
	
	initialize_gpio_for_input(21);	// rx iq data
	initialize_gpio_for_input(20);	// rx iq data
	initialize_gpio_for_input(19);	// rx iq data
	initialize_gpio_for_input(18);	// rx iq data
	
	//TX IO init.	
    initialize_gpio_for_input(12);
    initialize_gpio_for_output(4);
	clr_pin(4);
    initialize_gpio_for_output(5);
	clr_pin(5);

	printk(KERN_INFO "GPIO ready for using control and the rx and tx streaming...\n");
	
	return 0;
}


int rb2_trx_control(char *txBuf, char *rxBuf, unsigned cnt){
	
	struct spi_transfer t = {
		.tx_buf = txBuf,
		.rx_buf = rxBuf,
		.len = cnt,
	};
	struct spi_message m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	
	// Send and receive the control message to/from the Radioberry SPI device
	int ret = spi_sync(spi_ctrl_dev, &m);
	if (ret) {
		pr_err("SPI transfer failed\n");
		return ret;
	} 
	
   return 0;
}