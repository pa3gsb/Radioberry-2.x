CCFLAGS += -Wcast-function-type

KERNEL_HEADERS=/lib/modules/$(shell uname -r)/build

obj-m := radioberry.o
	
	
KERNEL_HEADERS=/lib/modules/$(shell uname -r)/build

obj-m := radioberry.o

all: radioberry dt	
	 echo Build Device Tree Overlay and kernel module.


radioberry:
	@$(MAKE) -C $(KERNEL_HEADERS) M=$(PWD) modules   

dt:
	dtc -@ -I dts -O dtb -o radioberry.dtbo radioberry.dts

clean:      
	@$(MAKE) -C $(KERNEL_HEADERS) M=$(PWD) clean

install: radioberry.ko
	cp radioberry_ioctl.h /usr/local/include
	sudo insmod radioberry.ko
	sudo chmod 666 /dev/radioberry

uninstall:
	sudo rmmod radioberry.ko

