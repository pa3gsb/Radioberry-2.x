/**
*
*	GPIO RPI-5 functions.
*
*/

#ifndef __RB2_RPI5_H__
#define __RB2_RPI5_H__

#include <linux/io.h>


#define GPIO ((GPIOregs*)GPIOBase)
#define PERI_BASE 0x1F00000000
#define MEM_SIZE (64 * 1024 * 1024)


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

extern  volatile uint32_t *PERIBase;
extern  volatile uint32_t *GPIOBase;
extern  volatile uint32_t *RIOBase;
extern  volatile uint32_t *PADBase;
extern  volatile uint32_t *SPIBase;

extern uint32_t *Pad;

#define rio ((rioregs *)RIOBase)
#define rioSET ((rioregs *)(RIOBase + 0x2000 / 4))
#define rioCLR ((rioregs *)(RIOBase + 0x3000 / 4))

#define GPIO_FUNC_RIO 5
#define PAD_FUNC_IN 	0xC8
#define PAD_FUNC_OUT 	0x10


int initialize_rpi(void); 

void deinitialize_rpi(void); 

void initialize_gpio_for_output(uint32_t pin); 

void initialize_gpio_for_input(uint32_t pin); 

void set_pin(uint32_t pin); 

void clr_pin(uint32_t pin); 

uint32_t read_pin(uint32_t pin); 

uint32_t read_pin_all(void); 

void setPinMode(uint32_t pin, uint32_t mode);

#endif
