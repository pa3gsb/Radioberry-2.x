/**
*
*	Loading the gateware into the FPGA.
*
*/


#include <linux/slab.h>
#include <linux/firmware.h>

#include "rb2-load-fpga.h"
#include "rb2-rpi5.h"


int prepare_gateware_loading(void) ;
void initialize_gateware(void);
int activate_gateware(void);


void upload_gateware_byte( int one_byte ) {
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

int prepare_gateware_loading() {
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

void initialize_gateware() {
	
	initialize_gpio_for_input(iPinCONF_DONE);
	initialize_gpio_for_input(iPinNSTATUS);
	
	initialize_gpio_for_output(oPinNCONFIG);
	initialize_gpio_for_output(oPinDATA);
	initialize_gpio_for_output(oPinDCLK);
	
	printk(KERN_INFO "GPIO ready for uploading gateware...\n");
	
}

int activate_gateware() {
		
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

void firmware_load(char *firmware, int size) {
	printk(KERN_INFO "inside %s function \n", __FUNCTION__);
	
	u8 *buf = kmalloc(size + 1, GFP_KERNEL);
	memcpy(buf, firmware, size);
	buf[size] = '\0';

	printk(KERN_INFO "Radioberry gateware file size: %d\n", size);

	initialize_gateware();
	prepare_gateware_loading();
	int b = 0;
	for (b = 0; b < size; b++) {
	   upload_gateware_byte(buf[b]);
	}
	activate_gateware();
	
	kfree(buf);
}

void loading_radioberry_gateware(struct device *dev) {
	printk(KERN_INFO "inside %s function \n", __FUNCTION__);
	
	const struct firmware *fw_entry;
	if (request_firmware(&fw_entry, "radioberry.rbf", dev) != 0 ) {
		   printk(KERN_ERR "gateware radioberry.rbf: Firmware not available\n");
		   return;
	}

	firmware_load(fw_entry->data, fw_entry->size);

	release_firmware(fw_entry);
}

// end of source