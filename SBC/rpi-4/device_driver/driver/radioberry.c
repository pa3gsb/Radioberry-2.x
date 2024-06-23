/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>

sudo cp radioberry.ko /lib/modules/$(uname -r)/kernel/drivers/sdr
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> 	
#include <linux/fs.h> 		
#include <linux/errno.h> 	
#include <linux/types.h> 	
#include <linux/proc_fs.h>
#include <linux/fcntl.h> 	
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/uaccess.h>	
#include <linux/kthread.h> 
#include <linux/sched.h>    
#include <linux/delay.h>
#include <linux/firmware.h>	
#include <linux/circ_buf.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/interrupt.h>            

static struct mutex spi_mutex;

#include "radioberry_rpi.h"
#include "radioberry_ioctl.h"
#include "radioberry_gateware.h"
#include "radioberry_firmware.h"


#define VERSION "4.01"
#define VERSION_INT 401

static DEFINE_MUTEX(radioberry_mutex); 
static wait_queue_head_t rx_sample_queue;

#define DEVICE_NAME "radioberry"   
#define DRIVER_NAME "radioberry"
#define CLASS_NAME  "radioberry"        

static int majorNumber;                  	
static struct class*  radioberryCharClass  = NULL; 
static struct device* radioberryCharDevice = NULL; 

#define SAMPLE_BYTES 512
static int _nrx = 1;

static unsigned int irqNumber; 
static struct gpio_desc *gpio_desc = NULL;

static irq_handler_t radioberry_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
	wake_up_interruptible(&rx_sample_queue);
    return (irq_handler_t) IRQ_HANDLED;      
}

static void firmware_load(const char *firmware, int size) {
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

static void loading_radioberry_gateware(struct device *dev) {
	printk(KERN_INFO "inside %s function \n", __FUNCTION__);
	
	const struct firmware *fw_entry;
	if (request_firmware(&fw_entry, "radioberry.rbf", dev) != 0 ) {
		   printk(KERN_ERR "gateware radioberry.rbf: Firmware not available\n");
		   return;
	}

	firmware_load(fw_entry->data, fw_entry->size);

	release_firmware(fw_entry);
}

ssize_t radioberry_read(struct file *flip, char *buf, size_t count, loff_t *pos) {
	unsigned char rx_stream[SAMPLE_BYTES]={};
	if ((((*rpi_read_io) >> 25) & 1) == 0)	wait_event_interruptible(rx_sample_queue, (((*rpi_read_io) >> 25) & 1) != 0);	
	count = rxStream(_nrx, rx_stream);  
	if (copy_to_user((char *)buf, &rx_stream, count)) return -EFAULT;
	return count;	
}

static ssize_t radioberry_write(struct file *flip, const char *buf, size_t count, loff_t *pos) {
 
 unsigned char tx_stream[4];

  if (count > 0) {
    if (copy_from_user(&tx_stream, buf, sizeof(tx_stream))) {
      return -EFAULT;
    }
	return write_iq_sample(tx_stream);
  }
  return 0;
}

static int radioberry_open(struct inode *inode, struct file *filep) {
	
	printk(KERN_INFO "inside %s function \n", __FUNCTION__);
	
	if(!mutex_trylock(&radioberry_mutex)){    										  
		  printk(KERN_ALERT "Radioberry Char: Device in use by another process");
		  return -EBUSY;
	}	
	int *minor = (int *)kmalloc(sizeof(int), GFP_KERNEL);
	// not used int major = MAJOR(inode->i_rdev);
	*minor = MINOR(inode->i_rdev);
	filep->private_data = (void *)minor;
	
	return 0;
}

static int radioberry_release(struct inode *inode, struct file *filep) {
	
	printk(KERN_INFO "inside %s function \n", __FUNCTION__);
	
	kfree(filep->private_data);
	mutex_unlock(&radioberry_mutex); 
	
	return 0;
}

static long radioberry_ioctl(struct file *fp, unsigned int cmd, unsigned long arg){
	
	//printk(KERN_INFO "inside %s function \n", __FUNCTION__);

	unsigned char data[6];
	int lnrx = _nrx;
	
	int rc;
	struct rb_info_arg_t *rb_info= kmalloc(sizeof(struct rb_info_arg_t), GFP_DMA);

	struct rb_info_arg_t rb_info_ret;

	switch(cmd){
		case RADIOBERRY_IOC_COMMAND:
			
			rc = copy_from_user(rb_info, (void *)arg, sizeof(struct rb_info_arg_t));
			
			data[0] = ( rb_info->rb_command           & 0xFF); //MSB  
			data[1] = ( rb_info->command              & 0xFF);
			data[2] = ((rb_info->command_data >> 24)  & 0xFF);
			data[3] = ((rb_info->command_data >> 16)  & 0xFF);
			data[4] = ((rb_info->command_data >>  8)  & 0xFF);
			data[5] = ( rb_info->command_data         & 0xFF);
			
			//printk(KERN_INFO "Command kernel %2X - %2X - %2X - %2X - %2X - %2X \n", data[0], data[1], data[2], data[3], data[4], data[5]);
			if ((data[1] & 0xFE)  == 0x00) lnrx = ((data[5] & 0x38) >> 3) + 1;
	
			mutex_lock(&spi_mutex);
			spiXfer(0, data, data, 6); //spi channel 0 // tell the gateware the command.
			mutex_unlock(&spi_mutex);
			
			_nrx = lnrx;
			
			// give feedback to firmware.
			rb_info_ret.rb_command = data[0]; // return the radioberry status information.
			rb_info_ret.major = data[4];
			rb_info_ret.minor = data[5];
			
			rb_info_ret.fpga = data[3] & 0x03; 
			rb_info_ret.version = VERSION_INT; 
			
			if (copy_to_user((struct rb_info_arg_t *)arg, &rb_info_ret, sizeof(struct rb_info_arg_t))) return -EACCES;
	
			break;

		default:
			return -ENOTTY;
	}

	return 0;
}

static struct file_operations radioberry_fops = {
	.owner 			= THIS_MODULE,
    .open 			= radioberry_open,
    .release 		= radioberry_release,
    .write 			= radioberry_write,
    .read 			= radioberry_read,
	.unlocked_ioctl = radioberry_ioctl
};

static int radioberry_probe(struct platform_device *pdev)
{
	int result;
	
	printk(KERN_INFO "inside %s function \n", __FUNCTION__);	
	if(!device_property_present(&pdev->dev, "rx-sample-gpio")) 
	{
		printk("radioberry - Error! Device overlay property 'rx-sample-gpio' not found!\n");
		return -1;
	}
	else
		printk("radioberry - Device property 'rx-sample-gpio' found!\n");
			
	
	// Dynamically allocate a major number for the device
	majorNumber = register_chrdev(0, DEVICE_NAME, &radioberry_fops);
	if (majorNumber<0){
	  printk(KERN_ALERT "Radioberry driver failed to register a major number\n");
	  return majorNumber;
	}
	printk(KERN_INFO "Radioberry: registered correctly with major number %d\n", majorNumber);

	// Register the device class
	radioberryCharClass = class_create(CLASS_NAME);
	if (IS_ERR(radioberryCharClass)){                
	  unregister_chrdev(majorNumber, DEVICE_NAME);
	  printk(KERN_ALERT "Failed to register device class\n");
	  return PTR_ERR(radioberryCharClass);         
	}
	printk(KERN_INFO "Radioberry: device class registered correctly\n");

	// Register the device driver
	radioberryCharDevice = device_create(radioberryCharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(radioberryCharDevice)){               
	  class_destroy(radioberryCharClass);           
	  unregister_chrdev(majorNumber, DEVICE_NAME);
	  printk(KERN_ALERT "Failed to create the device\n");
	  return PTR_ERR(radioberryCharDevice);
	}
	printk(KERN_INFO "Radioberry char: device class created correctly\n"); 
		
	gpio_desc = gpiod_get(&pdev->dev, "rx-sample", GPIOD_ASIS);
	if(IS_ERR(gpio_desc)) {
		printk(KERN_ALERT "Failed to get GPIO rx-sample-gpio\n");
		return -1 ;//* IS_ERR(gpio_desc);
	}
    printk("Got GPIO rx-sample-gpio\n");
	
	int retval = gpiod_direction_input(gpio_desc);
	if (retval) {
        printk("Failed to set GPIO pin direction\n");
        return retval;
    }
	irqNumber = gpiod_to_irq(gpio_desc);
    if (irqNumber < 0) {
        printk("Failed to get IRQ number for GPIO pin\n");
        return irqNumber;
    }
	printk(KERN_INFO "Radioberry: The rx samples pin is mapped to IRQ: %d\n", irqNumber);
	mutex_init(&radioberry_mutex);
	init_waitqueue_head(&rx_sample_queue);
	printk(KERN_INFO "Radioberry: The rx sample state is currently: %d\n", gpiod_get_value(gpio_desc));

	// GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
	// Get the IRQ number for the GPIO pin
	// This next call requests an interrupt line
	result = request_irq(irqNumber,             
						(irq_handler_t) radioberry_irq_handler, 
						IRQF_TRIGGER_RISING,   // Interrupt on rising edge  RQF_TRIGGER_RISING
						"radioberry_rx_irq",    // Used in /proc/interrupts to identify the owner
					NULL);

	printk(KERN_INFO "Radioberry: The interrupt request result is: %d\n", result);	
	mutex_init(&spi_mutex);
	initialize_rpi();
	loading_radioberry_gateware(radioberryCharDevice);
	initialize_firmware();
	return result;
}

static int radioberry_remove(struct platform_device *pdev)
{
	printk(KERN_INFO "inside %s function \n", __FUNCTION__);
	return 0;
}

static const struct of_device_id of_radioberry_match[] = {
		{.compatible = "sdr,radioberry", },
		{/*end of list */},
};

MODULE_DEVICE_TABLE(of, of_radioberry_match);

static struct platform_driver radioberry_driver = {
		.driver = {
				.name = DRIVER_NAME,
				.owner = THIS_MODULE,
				.of_match_table = of_match_ptr(of_radioberry_match),
		},
		.probe = radioberry_probe,
		.remove = radioberry_remove,
};

static int __init radioberry_init(void) {
	printk(KERN_INFO "inside %s function \n", __FUNCTION__);
	printk(KERN_INFO "%s loading...\n", DRIVER_NAME);

	int result = platform_driver_register(&radioberry_driver);
	printk(KERN_INFO "platform driver registered %d \n", result);	
	return result;
}

static void __exit radioberry_exit(void) {
	
	printk(KERN_INFO "inside %s function \n", __FUNCTION__);
	
	if (irqNumber > 0)
		free_irq(irqNumber, NULL);
    if (gpio_desc != NULL)
		gpiod_put(gpio_desc);
	
	platform_driver_unregister(&radioberry_driver);
	
	device_destroy(radioberryCharClass, MKDEV(majorNumber, 0));     
	class_unregister(radioberryCharClass);                        
	class_destroy(radioberryCharClass);                             
	unregister_chrdev(majorNumber, DEVICE_NAME); 
	
	mutex_destroy(&radioberry_mutex); 
	mutex_destroy(&spi_mutex);
	
	deinitialize_rpi();

	printk(KERN_INFO "Radioberry: Module removed!\n");	
}

module_init(radioberry_init);
module_exit(radioberry_exit);

/**
 *
 * Radioberry Driver Info
 *
 */
MODULE_AUTHOR("Johan Maas - pa3gsb@gmail.com");
MODULE_DESCRIPTION("Radioberry SDR device driver. (rpi-4)");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);