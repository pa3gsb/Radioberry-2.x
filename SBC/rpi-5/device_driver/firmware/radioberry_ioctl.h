#ifndef __RADIOBERRY_IOCTL_H__
#define __RADIOBERRY_IOCTL_H__

#include <linux/types.h>
#include <linux/ioctl.h>

#define RADIOBERRY_MAGIC	('x')

#define RADIOBERRY_IOC_COMMAND			_IOW(RADIOBERRY_MAGIC, 1, __u8)

struct rb_info_arg_t
{
	/* gateware info */
    int major;					/* gateware major version */
	int minor;  				/* gateware minor version */
	int fpga;   				/* gateware fpga type used by radioberry board */
	int nr; 					/* gateware number of implemented receivers */
	int nt;						/* gateware number of implemented transmitters */
	
	/* driver info */
	int version; 				/*driver version*/
	
	/* protocol commands */
	int rb_command;
	int command;
	int command_data;
	
} ;

struct rb_info_arg_t rb_info;

#endif
