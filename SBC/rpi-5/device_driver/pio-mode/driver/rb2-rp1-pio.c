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
	
#include <linux/platform_device.h>  
#include <linux/spi/spi.h>
#include <linux/kfifo.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
								
#include "rb2-rp1-pio.h"
#include "rb2-rpi5.h"
#include "rb2-load-fpga.h"
#include "rb2-trx-control.h"
#include "rb2-rx-stream.h"
#include "rb2-tx-stream.h"

#include "radioberry_ioctl.h"

#define VERSION "5.51"
#define VERSION_INT 551

static DEFINE_MUTEX(radioberry_mutex); 

#define DEVICE_NAME "radioberry"   
#define DRIVER_NAME "radioberry"
#define CLASS_NAME  "radioberry"        

static int majorNumber;                  	
static struct class*  radioberryCharClass  = NULL; 
static struct device* radioberryCharDevice = NULL; 

static int _nrx = 1;

static struct radioberry_client_ctx *gctx;
static bool gctx_ready;

static int rb2_rp1_pio_init(struct device *dev)
{
    int ret;

    if (gctx_ready) return 0;

    gctx = kzalloc(sizeof(*gctx), GFP_KERNEL);
    if (!gctx) return -ENOMEM;

    ret = configure_rx_iq_sm(gctx);
    if (ret) goto err_free;

    ret = configure_tx_iq_sm(gctx);
    if (ret) goto err_rx;

    gctx_ready = true;
	
    pr_info("radioberry: RX SM %d and TX SM %d initialised.\n", gctx->rx.sm, gctx->tx.sm);
    return 0;

err_rx:
    rp1_pio_close(gctx->rx.client);
err_free:
    kfree(gctx);
    gctx = NULL;
    return ret;
}

static void rb2_rp1_pio_deinit(void)
{
    if (!gctx) return;
	
    radioberry_cleanup_rx_ctx(gctx);
    radioberry_cleanup_tx_ctx(gctx);

    kfree(gctx);
    gctx = NULL;
    gctx_ready = false;
}


static int spi_ctrl_probe(struct spi_device *spi)
{
	printk(KERN_INFO "inside %s function \n", __FUNCTION__);

	spi_ctrl_dev = spi;
	  
    int ret = spi_setup(spi);
    if (ret) {
        pr_err("Failed to set up SPI communication\n");
        return ret;
    }
    return 0;
}

// Declare the SPI driver structure
static struct spi_driver radioberry_spi_ctrl_driver = {
    .driver = {
        .name = "radioberry_ctrl_spi",  
        .owner = THIS_MODULE,
    },
    .probe = spi_ctrl_probe,
};

ssize_t radioberry_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct radioberry_client_ctx *ctx = file->private_data;
    
	unsigned int copied;
	uint8_t read_buf[512]; 
	
	int nr_samples = (_nrx == 1)? 63 : (_nrx == 2)? 72: (_nrx ==3)? 75: (_nrx ==4)? 76: (_nrx ==5)? 75: (_nrx ==6)? 78: (_nrx ==7)? 77: 80;
	count = nr_samples * 6;
	long timeout_jiffies = msecs_to_jiffies(1000); 

	if (wait_event_interruptible_timeout(ctx->rx.queue,
										 kfifo_len(&ctx->rx.dma_fifo) >= count,
										 timeout_jiffies) == 0) {
		pr_warn("radioberry_read: timeout waiting for data\n");
		return -EAGAIN; 
	}
	
	unsigned long flags;
	spin_lock_irqsave(&ctx->rx.fifo_lock, flags);
		copied = kfifo_out(&ctx->rx.dma_fifo, read_buf, count);
	spin_unlock_irqrestore(&ctx->rx.fifo_lock, flags);
	
    if (copied == 0) {
        pr_err("radioberry_read: no data copied from fifo despite readiness (requested=%zu, available=%u)\n",
               count, kfifo_len(&ctx->rx.dma_fifo));
        return -EIO;
    }		

	if (copy_to_user((char *)buf, &read_buf, copied)) return -EFAULT;

    return copied;
}

ssize_t radioberry_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	struct radioberry_client_ctx *ctx = file->private_data;
	struct radioberry_stream *tx = &ctx->tx;

    uint8_t *tx_stream;
    tx_stream = kmalloc(len, GFP_KERNEL);
    if (!tx_stream)
        return -ENOMEM;
    if (copy_from_user(tx_stream, buf, len)) {
        kfree(tx_stream);
        return -EFAULT;
    }
	long timeout_jiffies = msecs_to_jiffies(1000); 

	if (wait_event_interruptible_timeout(ctx->tx.queue,
										 kfifo_avail(&ctx->tx.dma_fifo) >= len,
										 timeout_jiffies) == 0) {
		pr_warn("radioberry_write: timeout waiting to write data\n");
		return -EAGAIN; 
	}
	
    ssize_t written = rb2_tx_stream_write(tx, tx_stream, len);
    kfree(tx_stream);
    return written;
}

static int radioberry_open(struct inode *inode, struct file *filep)
{
    printk(KERN_INFO "inside %s function \n", __FUNCTION__);

    if (!mutex_trylock(&radioberry_mutex)) {
        printk(KERN_ALERT "Radioberry Char: Device in use by another process");
        return -EBUSY;
    }
    if (!gctx_ready) {
        mutex_unlock(&radioberry_mutex);
        return -ENODEV;
    }

    unsigned long flags;
    spin_lock_irqsave(&gctx->rx.fifo_lock, flags);
    kfifo_reset(&gctx->rx.dma_fifo);
    spin_unlock_irqrestore(&gctx->rx.fifo_lock, flags);
	spin_lock_irqsave(&gctx->tx.fifo_lock, flags);
    kfifo_reset(&gctx->tx.dma_fifo);
    spin_unlock_irqrestore(&gctx->tx.fifo_lock, flags);

    filep->private_data = gctx;

    pr_info("radioberry: streaming enabled (RX SM %d, TX SM %d)\n", gctx->rx.sm, gctx->tx.sm);
    return 0;
}

static int radioberry_release(struct inode *inode, struct file *filep)
{
    printk(KERN_INFO "inside %s function \n", __FUNCTION__);

    struct radioberry_client_ctx *ctx = filep->private_data;

    if (ctx) {
        filep->private_data = NULL;
    }

    mutex_unlock(&radioberry_mutex);
    return 0;
}

static long radioberry_ioctl(struct file *fp, unsigned int cmd, unsigned long arg){
	
	//printk(KERN_INFO "inside %s function \n", __FUNCTION__);

	unsigned char data[6];
	int lnrx = _nrx;
	
	int rc;
	struct rb_info_arg_t *rb_info= kmalloc(sizeof(struct rb_info_arg_t), GFP_KERNEL );

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
	
			rb2_trx_control(data, data, 6); //spi channel 0 // tell the gateware the command.

			_nrx = lnrx;
			
			//printk(KERN_INFO "SDR info       %2X - %2X - %2X - %2X - %2X - %2X \n", data[0], data[1], data[2], data[3], data[4], data[5]);
			
			// give feedback to firmware.
			rb_info_ret.rb_command = data[0]; // return the radioberry status information.
			rb_info_ret.major = data[4];
			rb_info_ret.minor = data[5];
			
			rb_info_ret.fpga = data[3] & 0x03; 
			rb_info_ret.version = VERSION_INT; 
			
			if (copy_to_user((struct rb_info_arg_t *)arg, &rb_info_ret, sizeof(struct rb_info_arg_t))) return -EACCES;
	
			break;
		default:
			kfree(rb_info);
			return -ENOTTY;
	}
	kfree(rb_info);
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
	printk(KERN_INFO "inside %s function \n", __FUNCTION__);
		
	struct device *dev = &pdev->dev;	
    struct device_node *pio_np;
    struct platform_device *pio_pdev;

    /* Fetch phandle to RP1-PIO from device tree */
    pio_np = of_parse_phandle(dev->of_node, "pio", 0);
    if (!pio_np) {
		printk(KERN_INFO "Failed to get 'pio' phandle \n");
        dev_err(dev, "Failed to get 'pio' phandle\n");
        return -ENODEV;
    }

    pio_pdev = of_find_device_by_node(pio_np);
    of_node_put(pio_np);
    if (!pio_pdev) {
		printk(KERN_INFO "Failed to find RP1-PIO platform device \n");
        dev_err(dev, "Failed to find RP1-PIO platform device\n");
        return -ENODEV;
    }

    /* Fetch rp1_pio_device struct */
    struct rp1_pio_device *pio = dev_get_drvdata(&pio_pdev->dev);
    if (!pio) {
		printk(KERN_INFO "Failed to get RP1-PIO drvdata \n");
        dev_err(dev, "Failed to get RP1-PIO drvdata\n");
        return -ENODEV;
    }

	platform_set_drvdata(pdev, pio);  

	dev_info(dev, "Radioberry probe finished\n");
	
	return 0;
}

static const struct of_device_id of_radioberry_match[] = {
		{.compatible = "sdr,radioberry" },
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
};

static int __init radioberry_init(void) {
	int retval;
	size_t size;

	printk(KERN_INFO "inside %s function \n", __FUNCTION__);
	printk(KERN_INFO "%s loading...\n", DRIVER_NAME);

    // Register the SPI driver
    int ret = spi_register_driver(&radioberry_spi_ctrl_driver);
    if (ret != 0) {
        pr_err("Failed to register SPI driver\n");
        return ret;
    }
	printk(KERN_INFO "spi register driver executed with result %d\n", ret);

	int result = platform_driver_register(&radioberry_driver);
	printk(KERN_INFO "platform driver registered %d \n", result);

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
	
	mutex_init(&radioberry_mutex);

	initialize_rpi();
	loading_radioberry_gateware(radioberryCharDevice); 
	rb2_trx_initialize();
	
    ret = rb2_rp1_pio_init(radioberryCharDevice);
    if (ret) {
		pr_err("radioberry: failed to claim PIO SMs: %d\n", ret);
		return ret;
    }
	
	return result;
}

static void __exit radioberry_exit(void) {

	printk(KERN_INFO "inside %s function \n", __FUNCTION__);
		
	platform_driver_unregister(&radioberry_driver);
	
	device_destroy(radioberryCharClass, MKDEV(majorNumber, 0));     
	class_unregister(radioberryCharClass);                        
	class_destroy(radioberryCharClass);                             
	unregister_chrdev(majorNumber, DEVICE_NAME); 
	
	mutex_destroy(&radioberry_mutex); 

    // Unregister the SPI driver
    spi_unregister_driver(&radioberry_spi_ctrl_driver);
	
	deinitialize_rpi();
	
	rb2_rp1_pio_deinit();

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
MODULE_DESCRIPTION("Radioberry SDR device driver. (rpi-5)");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);