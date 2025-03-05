#!/bin/bash
echo ""
echo "This docker image builds the radioberry driver for the pi-4."
echo "" 
echo ""
echo ""
echo "kernel header source files are searched at location: /usr/src/linux-headers-$(uname -r) "
if [ ! -d /usr/src/linux-headers-$(uname -r) ]; then
    echo "Kernel header source files $(uname -r) are not available."
	echo "Install the headers : run this command in a shell: 'sudo apt-get install raspberrypi-kernel-headers'"
    exit 1
fi
echo "kernel header source files are present"

echo ""
echo "kernel modules must be present at location:  /lib/modules/$(uname -r)"
if [ ! -d /lib/modules/$(uname -r) ]; then
    echo "Kernel modules $(uname -r) are not available."
    exit 1
fi
echo "kernel modules are present"
echo ""



echo ""
echo "Building kernel...."
make all > /dev/null 2>&1


# Check build proces.
if [ $? -eq 0 ]; then
	mkdir -p /tmp/radioberry_kernel_pi4
    cp *.ko *.dtbo /tmp/radioberry_kernel_pi4
	echo "Radioberry driver built successfull, brilliant!"
	echo "Result, driver can be found in: /tmp/radioberry_kernel_pi4"
else
    echo "Radioberry driver built failed"
    exit 1
fi

exit 0