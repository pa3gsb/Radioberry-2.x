KERNEL_HEADERS ?= /lib/modules/$(shell uname -r)/build

KBUILD_EXTRA_SYMBOLS := /home/pi/git/rp1-driver/Module.symvers

ccflags-y += -I$(PWD)/include


obj-m := radioberry.o 

radioberry-objs := rb2-rp1-pio.o src/rb2-rpi5.o src/rb2-load-fpga.o src/rb2-trx-control.o src/rb2-rx-stream.o src/rb2-tx-stream.o 




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

