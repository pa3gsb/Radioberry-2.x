/*

Raspberry Pi 5 

GPIO support.

*/


#include "rb2-rpi5.h"

volatile uint32_t *PERIBase = NULL;
volatile uint32_t *GPIOBase = NULL;
volatile uint32_t *RIOBase = NULL;
volatile uint32_t *PADBase = NULL;

uint32_t *Pad = NULL;


int initialize_rpi(void) {
	
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
	
	printk(KERN_INFO "GPIO ready for use\n");
	
	return ret;
}

void deinitialize_rpi(void) {	
	iounmap(PERIBase);
	printk(KERN_INFO "GPIO resources free. \n");
}

void initialize_gpio_for_output(uint32_t pin) {
    GPIO[pin].ctrl=GPIO_FUNC_RIO;
    Pad[pin] = PAD_FUNC_OUT;
    rioSET->oe = 0x01<<pin; 	// output driver
}

void initialize_gpio_for_input(uint32_t pin) {
	GPIO[pin].ctrl=GPIO_FUNC_RIO;
    Pad[pin] = PAD_FUNC_IN;
    rioCLR->oe = 0x01<<pin; 	// high impedance
}

void set_pin(uint32_t pin) {
	rioSET->oe = 0x01<<pin; 	// output driver
	rioSET->out = 0x01<<pin;
}

void clr_pin(uint32_t pin) {
	rioSET->oe  = 0x01<<pin; 	// output driver
	rioCLR->out = 0x01<<pin;
}

uint32_t read_pin(uint32_t pin) {
	return (rio->in>>pin & 0x01);
}

uint32_t read_pin_all(void) {
	return rio->in;
}

void setPinMode(uint32_t pin, uint32_t mode)
{
	GPIO[pin].ctrl = mode;
}