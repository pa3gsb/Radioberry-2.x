CCFLAGS += -Wcast-function-type

KERNEL_HEADERS=/lib/modules/$(shell uname -r)/build

obj-m := radioberry.o

radioberry.ko:
	@$(MAKE) -C $(KERNEL_HEADERS) M=$(PWD) modules

clean:      
	@$(MAKE) -C $(KERNEL_HEADERS) M=$(PWD) clean

install: radioberry.ko
	cp radioberry_ioctl.h /usr/local/include
	sudo insmod radioberry.ko
	sudo chmod 666 /dev/radioberry

uninstall:
	sudo rmmod radioberry.ko

