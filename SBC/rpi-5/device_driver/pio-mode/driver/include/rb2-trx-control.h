#ifndef __RB2_TRX_CONTROL_H__
#define __RB2_TRX_CONTROL_H__

#define GPIO_FUNC_SPI 0  
#define RPI_RX_CLK 6

#define RPI_SPI_CE0   8
#define RPI_SPI_CE1   7
#define RPI_SPI_SCLK 11
#define RPI_SPI_MISO  9
#define RPI_SPI_MOSI 10


#include <linux/spi/spi.h>

extern struct spi_device *spi_ctrl_dev;

int rb2_trx_initialize(void);

int rb2_trx_control(char *txBuf, char *rxBuf, unsigned cnt);


#endif

