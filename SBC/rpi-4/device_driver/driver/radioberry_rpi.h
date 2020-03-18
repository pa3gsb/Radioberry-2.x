/**
*
*	GPIO RPI-4 functions.
*
*/

#ifndef __RADIOBERRY_RPI_H__
#define __RADIOBERRY_RPI_H__


#define GPIO_SET_OFFSET 0x1C
#define GPIO_CLR_OFFSET 0x28
#define GPIO_LEV0_OFFSET 0x34

#define BCM2711_PI4_PERI_BASE  0xFE000000

#define RPI_REG_BASE BCM2711_PI4_PERI_BASE

#define RPI_GPIO_OFFSET	 0x200000
#define RPI_SPI_OFFSET	 0x204000
#define RPI_GPIO_SIZE 	0xC0
#define RPI_GPIO_BASE 	(RPI_REG_BASE + RPI_GPIO_OFFSET)
#define RPI_SPI_BASE  	(RPI_REG_BASE + RPI_SPI_OFFSET)

#define SPI_LEN   0x18


static int initialize_rpi(void);
static void deinitialize_rpi(void);

volatile uint32_t *rpi_io=NULL; 
volatile uint32_t *rpi_set_io_high=NULL; 
volatile uint32_t *rpi_set_io_low=NULL;
volatile uint32_t *rpi_read_io=NULL; 

volatile uint32_t *spiReg  = NULL;

void initialize_gpio_for_output(volatile uint32_t *gpio_registerset, int bit) {
  *(gpio_registerset+(bit/10)) &= ~(7<<((bit%10)*3));  // prepare: set as input
  *(gpio_registerset+(bit/10)) |=  (1<<((bit%10)*3));  // set as output.
}

void initialize_gpio_for_input(volatile uint32_t *gpio_registerset, int bit) {
  *(gpio_registerset+(bit/10)) &= ~(7<<((bit%10)*3));  // prepare: set as input
}

static void setPinMode(unsigned gpio, unsigned mode)
{
   int reg, shift;

   reg   =  gpio/10;
   shift = (gpio%10) * 3;

   rpi_io[reg] = (rpi_io[reg] & ~(7<<shift)) | (mode<<shift);
}

static int initialize_rpi() {
	
	int ret = 0;
	 
	if((rpi_io = ioremap(RPI_GPIO_BASE, 0x60)) == NULL){
		printk(KERN_INFO "Failed mapping gpio registers...\n");
		ret = -1;
	}
	
	if((spiReg = ioremap(RPI_SPI_BASE, SPI_LEN)) == NULL){
		printk(KERN_INFO "Failed mapping SPI registers...\n");
		ret = -1;
	}
	
	rpi_set_io_high = rpi_io + (GPIO_SET_OFFSET / sizeof(uint32_t));
	rpi_set_io_low = rpi_io + (GPIO_CLR_OFFSET / sizeof(uint32_t));
	rpi_read_io= rpi_io + (GPIO_LEV0_OFFSET / sizeof(uint32_t));

	
	printk(KERN_INFO "GPIO ready for use\n");
	
	return ret;
	
}

static void deinitialize_rpi() {	
	iounmap(rpi_io);
	iounmap(spiReg);
	printk(KERN_INFO "GPIO resources free. \n");
}


#endif
