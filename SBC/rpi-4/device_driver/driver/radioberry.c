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

*/

// ++++   Device driver for the radioberry running at a rpi-4. ++++

#include <linux/init.h>
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

#include "radioberry_rpi.h"
#include "radioberry_ioctl.h"
#include "radioberry_gateware.h"
#include "radioberry_firmware.h"

#define DEVNAME_RADIOBERRY "radioberry"
#define NUM_DEV_RB 1
#define SAMPLE_BYTES 512

#define DEV_MAJOR 0
#define DEV_MINOR 0
static int _major_radioberry = DEV_MAJOR;
static int _minor_radioberry = DEV_MINOR;

static int _nrx = 1;

static struct cdev *cdev_array = NULL;
static struct class *class_rb = NULL;

static volatile int cdev_index = 0;

static struct device *radioberry_device;

static void firmware_load(char *firmware, int size) {
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
       const struct firmware *fw_entry;

       if (request_firmware(&fw_entry, "radioberry.rbf", dev) != 0 ) {
               printk(KERN_ERR "gateware radioberry.rbf: Firmware not available\n");
               return;
       }

       firmware_load(fw_entry->data, fw_entry->size);

       release_firmware(fw_entry);
}

ssize_t radioberry_read(struct file *flip, char *buf, size_t count, loff_t *pos) {
	
	unsigned char rx_stream[SAMPLE_BYTES];		 
	count = rxStream(_nrx, rx_stream);
	
	if (count) {if (copy_to_user((char *)buf, &rx_stream, count)) return -EFAULT;}

  return count;
}

static int radioberry_write(struct file *flip, const char *buf, size_t count, loff_t *pos) {
 
 unsigned char tx_stream[4];

  if (count > 0) {
    if (copy_from_user(&tx_stream, buf, sizeof(tx_stream))) {
      return -EFAULT;
    }
	write_iq_sample(tx_stream);
    return count;
  }
  return 0;
}

static int radioberry_open(struct inode *inode, struct file *filep) {
  int *minor = (int *)kmalloc(sizeof(int), GFP_KERNEL);
  int major = MAJOR(inode->i_rdev);
  *minor = MINOR(inode->i_rdev);

  filep->private_data = (void *)minor;
  
  return 0;
}

static int radioberry_release(struct inode *inode, struct file *filep) {
  kfree(filep->private_data);
  return 0;
}

static long radioberry_ioctl(struct file *fp, unsigned int cmd, unsigned long arg){

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
			printk(KERN_INFO "Aantal nrx %d \n", lnrx);
	
			// tell the gateware the command.
			spiXfer(data, data, 6);
			
			_nrx = lnrx;
			
			// give feedback to firmware.
			rb_info_ret.major = data[4];
			rb_info_ret.minor = data[5];
			
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

static int radioberry_register_dev(void) {
	int retval;
	dev_t dev;
	dev_t devno;
	int i;

	retval = alloc_chrdev_region(&dev, DEV_MINOR, NUM_DEV_RB, DEVNAME_RADIOBERRY);

	if (retval < 0) {
	printk(KERN_ERR "alloc_chrdev_region failed.\n");
		return retval;
	}
	_major_radioberry = MAJOR(dev);

	class_rb = class_create(THIS_MODULE, DEVNAME_RADIOBERRY);
	if (IS_ERR(class_rb)) {
		return PTR_ERR(class_rb);
	}
	
	for (i = 0; i < NUM_DEV_RB; i++) {
		devno = MKDEV(_major_radioberry, _minor_radioberry + i);
		cdev_init(&(cdev_array[cdev_index]), &radioberry_fops);
		cdev_array[cdev_index].owner = THIS_MODULE;
		if (cdev_add(&(cdev_array[cdev_index]), devno, 1) < 0) {
		  printk(KERN_ERR "cdev_add failed minor = %d\n", _minor_radioberry + i);
		} else {
		  radioberry_device = device_create(class_rb, NULL, devno, NULL, DEVNAME_RADIOBERRY, NULL);
		}
		cdev_index++;
	}
  
  return 0;
}

static int radioberry_init(void) {
	int retval;
	size_t size;

	printk(KERN_INFO "%s loading...\n", DEVNAME_RADIOBERRY);

	size = sizeof(struct cdev) * NUM_DEV_RB;
	cdev_array = (struct cdev *)kmalloc(size, GFP_KERNEL);

	retval = radioberry_register_dev();
	if (retval != 0) {
		printk(KERN_ALERT " Radioberry driver register failed.\n");
		return retval;
	}
	
	initialize_rpi();
	loading_radioberry_gateware(radioberry_device);
	initialize_firmware();
  	
	return 0;
}

static void radioberry_exit(void) {
	int i;
	dev_t devno;
	dev_t devno_top;
	
	deinitialize_rpi();

	for (i = 0; i < NUM_DEV_RB; i++) {
		cdev_del(&(cdev_array[i]));
	}

	devno_top = MKDEV(_major_radioberry, _minor_radioberry);
	for (i = 0; i < NUM_DEV_RB; i++) {
		devno = MKDEV(_major_radioberry, _minor_radioberry + i);
		device_destroy(class_rb, devno);
	}
	unregister_chrdev_region(devno_top, NUM_DEV_RB);

	class_destroy(class_rb);

	kfree(cdev_array);
	printk("module being removed at %lu\n", jiffies);
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
MODULE_SUPPORTED_DEVICE("Radioberry SDR");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");