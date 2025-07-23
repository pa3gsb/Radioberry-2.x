## Radioberry Device Driver


This device driver is a special for the RPI-5.


Build step:

First get the linux headers:

sudo apt-get install raspberrypi-kernel-headers

Second you need to start the build process:

use in a command window, in the folder driver (which contains the Makefile) : make

This will results in a radioberry.ko file and radioberry.dtbo file.



Hereby a step by step setup:


Step -1-

CL025 FPGA Radioberry users:
Copy the gateware radioberry.rbf into the folder /lib/firmware

CL016 FPGA Radioberry users:
Rename your radioberry-10CL016.rbf gateware to radioberry.rbf 
Copy the gateware radioberry.rbf into the folder /lib/firmware

Loading the device driver will also load the gateware (content of the rbf file) into the FPGA 



Step -2-

Make a module folder in the driver area of your running kernel, and install the driver called radioberry.ko in this folder.

cd /lib/modules/$(uname -r)/kernel/drivers

sudo mkdir sdr

copy the radioberry.ko to /lib/modules/$(uname -r)/kernel/drivers/sdr using the command:

sudo cp radioberry.ko /lib/modules/$(uname -r)/kernel/drivers/sdr



Step -3-

run the command:  sudo depmod 

Check:  use the command: sudo modinfo radioberry 

Gives you detailed info about the radioberry device driver.



Step -4-

load the device driver:

run the command sudo dtoverlay radioberry.dtbo

run the command: sudo modprobe radioberry

In the folder /dev the radioberry must be present using the ls command.
Also possible to check by the command: lsmod |grep radioberry


Step -5-

Optional step.


cp this radioberry.dtbo into /boot/overlays

add the following line in config.txt:

(location /boot/firmware/config.txt)

dtoverlay=radioberry

This loads the kernel module during boot.



Step -6-

Optional step.

sudo chmod 666 /dev/radioberry

Makes it possible to run the radioberry firmware version for the device driver, running as the logged in user:



Step -7-

Run using the command ./radioberry firmware or sudo ./radioberry


Step -8-
Start a SDR program!


Have fun listening to your Radioberry using the radioberry device driver.

73 Johan
PA3GSB


