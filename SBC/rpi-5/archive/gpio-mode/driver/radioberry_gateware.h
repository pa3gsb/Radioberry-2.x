/**
*
*	Loading the gateware into the FPGA.
*
*/

#ifndef __RADIOBERRY_GATEWARE_H__
#define __RADIOBERRY_GATEWARE_H__

#include <linux/delay.h>


#define iPinCONF_DONE 		22	
#define iPinNSTATUS 		26	

#define oPinNCONFIG 		27	
#define oPinDATA 			13	
#define oPinDCLK 			24

static void initialize_gateware(void);
static int prepare_gateware_loading(void) ;
static void upload_gateware_byte( int one_byte );
static int activate_gateware(void);

static void upload_gateware_byte( int one_byte ) {
	int	bit = 0;
	int i = 0;

	/* write from LSb to MSb */
	for ( i = 0; i < 8; i++ )
	{
		bit = one_byte >> i;
		bit = bit & 0x1;
		
		if (bit) set_pin(oPinDATA); else clr_pin(oPinDATA); 
		udelay(1);
		set_pin(oPinDCLK);
		clr_pin(oPinDCLK);
	}
}

static int prepare_gateware_loading() {
	printk(KERN_INFO "Info: prepare FPGA for loading image\n");
	
	clr_pin(oPinNCONFIG);
	clr_pin(oPinDATA);
	clr_pin(oPinDCLK);
 
	msleep(1000);
	
	set_pin(oPinNCONFIG);

	int count = 0;
	
	while (read_pin(iPinNSTATUS) == 0) {
		count++;
		msleep(1000);
		if (count >= 2) {
			printk(KERN_INFO "Error: prepareLoading failed\n");
			return -1;
		}
	}
	
	return 0;
}

static void initialize_gateware() {
	
	initialize_gpio_for_input(iPinCONF_DONE);
	initialize_gpio_for_input(iPinNSTATUS);
	
	initialize_gpio_for_output(oPinNCONFIG);
	initialize_gpio_for_output(oPinDATA);
	initialize_gpio_for_output(oPinDCLK);
	
	printk(KERN_INFO "GPIO ready for uploading gateware...\n");
	
}

static int activate_gateware() {
		
	/* Check if loading succeeded*/
	if (read_pin(iPinNSTATUS) == 0) {
		printk(KERN_INFO "Error: programming failed; nstatus is low\n");
		return 0;
	} else if (read_pin(iPinCONF_DONE) == 0) {
			printk(KERN_INFO "Error: programming failed; conf done is low\n");
			return 0;
	}
		
	/* Initialize device */
	set_pin(oPinDCLK);
	clr_pin(oPinDCLK);
	set_pin(oPinDCLK);
	clr_pin(oPinDCLK);

	printk(KERN_INFO "Info: gateware upload and activation succeeded\n");
	
	return 0;
}

#endif

