#ifndef __RADIOBERRY_IOCTL_H__
#define __RADIOBERRY_IOCTL_H__

#include <linux/types.h>
#include <linux/ioctl.h>

#define RADIOBERRY_MAGIC	('x')

#define RADIOBERRY_IOC_COMMAND			_IOW(RADIOBERRY_MAGIC, 1, __u8)

struct rb_info_arg_t
{
    int major, minor;
	
	int fpga;
	
	int version;
	
	int rb_command;
	int command;
	int command_data;
	
} ;

#endif
