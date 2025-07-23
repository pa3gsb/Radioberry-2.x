/**
*
*	GPIO RPI-5 functions.
*
*/

#ifndef __RADIOBERRY_RPI_H__
#define __RADIOBERRY_RPI_H__

#define GPIO ((GPIOregs*)GPIOBase)


#define PERI_BASE 0x1F00000000
#define MEM_SIZE (64 * 1024 * 1024)

#define RP1_SPI0_BASE 0x050000

#define DW_SPI_CTRLR0 0x00	// control register 0 (frame format, clock polarity, phase, etc.) 
#define DW_SPI_SSIENR 0x04 	// enable register (enable/disable the SPI controller) (0x08)
#define DW_SPI_SER 0x04     // chip select, bit = chip select line (0x10)
#define DW_SPI_BAUDR 0x05 	// baud rate register (0x14)
#define DW_SPI_SR 0x0a    	// status register (0x28)
#define DW_SPI_DR 0x18 		// data register (0x60)

//                                    10987654321098765432109876543210
#define DW_PSSI_CTRLR0_DFS_MASK     0b00000000000000000000000000001111 //GENMASK(3, 0)
#define DW_PSSI_CTRLR0_DFS32_MASK   0b00000000000111110000000000000000 //GENMASK(20, 16)

//SPI
#define DW_PSSI_CTRLR0_MODE_MASK    0b00000000000000000000000011000000 //GENMASK(7, 6)
#define DW_PSSI_CTRLR0_SCPHA        0b00000000000000000000000001000000 // BIT(6)
#define DW_PSSI_CTRLR0_SCPOL        0b00000000000000000000000010000000 //BIT(7)

/* Bit fields in SR, 7 bits */
#define DW_SPI_SR_TF_NOT_FULL       0b00000000000000000000000000000010 // BIT(1)
#define DW_SPI_SR_RF_NOT_EMPT       0b00000000000000000000000000001000 // BIT(3)


typedef struct{
    uint32_t status;
    uint32_t ctrl; 
}	GPIOregs;


typedef struct
{
    uint32_t out;
    uint32_t oe;
    uint32_t in;
    uint32_t inSync;
} rioregs;

volatile uint32_t *PERIBase = NULL;
volatile uint32_t *GPIOBase = NULL;
volatile uint32_t *RIOBase  = NULL;
volatile uint32_t *PADBase  = NULL;
volatile uint32_t *SPIBase 	= NULL;

uint32_t *Pad = NULL;

#define rio ((rioregs *)RIOBase)
#define rioSET ((rioregs *)(RIOBase + 0x2000 / 4))
#define rioCLR ((rioregs *)(RIOBase + 0x3000 / 4))

#define GPIO_FUNC_RIO 5
#define PAD_FUNC_IN 	0xC8
#define PAD_FUNC_OUT 	0x10


static int initialize_rpi(void) {
	
	int ret = 0;
	
	printk(KERN_INFO "make GPIO ready for use\n");
		 
	if((PERIBase = ioremap(PERI_BASE, MEM_SIZE)) == NULL){
		printk(KERN_INFO "Failed mapping registers...\n");
		ret = -1;
	}
	
	GPIOBase = PERIBase + 0xD0000 / 4;
	RIOBase  = PERIBase + 0xE0000 / 4;
	PADBase  = PERIBase + 0xF0000 / 4;
	Pad = PADBase + 1;
	SPIBase  = PERIBase + RP1_SPI0_BASE / 4;
	

	printk(KERN_INFO "GPIO ready for use\n");
	
	return ret;
}

static void deinitialize_rpi(void) {	
	iounmap(PERIBase);
	printk(KERN_INFO "GPIO resources free. \n");
}

static void initialize_gpio_for_output(uint32_t pin) {
    GPIO[pin].ctrl=GPIO_FUNC_RIO;
    Pad[pin] = PAD_FUNC_OUT;
    rioSET->oe = 0x01<<pin; 	// output driver
}

static void initialize_gpio_for_input(uint32_t pin) {
	GPIO[pin].ctrl=GPIO_FUNC_RIO;
    Pad[pin] = PAD_FUNC_IN;
    rioCLR->oe = 0x01<<pin; 	// high impedance
}

static void set_pin(uint32_t pin) {
	rioSET->oe = 0x01<<pin; 	// output driver
	rioSET->out = 0x01<<pin;
}

static void clr_pin(uint32_t pin) {
	rioSET->oe  = 0x01<<pin; 	// output driver
	rioCLR->out = 0x01<<pin;
}

static uint32_t read_pin(uint32_t pin) {
	return (rio->in>>pin & 0x01);
}

static uint32_t read_pin_all(void) {
	return rio->in;
}

static void setPinMode(uint32_t pin, uint32_t mode)
{
	GPIO[pin].ctrl = mode;
}

#endif
