#!/bin/bash

echo ""
echo ""
echo "============================================"
echo ""
echo "Radioberry driver software installation."
echo ""
echo ""
echo "if you are facing issues or having ideas for improvement use the forum."
echo "The forum can be found at https://groups.google.com/g/radioberry"
echo ""
echo "Have fun"
echo "Johan, PA3GSB"
echo "============================================"
echo ""

#[ $(id -u) = 0 ] ; echo "Please do not run this script as root" : exit 1

if [ "$(getconf LONG_BIT)" -eq 64 ]; then
    echo "Greate you are running at a 64-bit system. "
else
    echo "This is not a 64 bit system; Please make use of a 64 bit Linux variant."
	exit 1
fi


if [ -f /lib/firmware/radioberry.rbf ]; then
  echo "Radioberry gateware (radioberry.rbf) is present in /lib/firmware folder."
else
	echo "Radioberry card uses a FPGA chip. There are 2 variants available."
	echo "During your own built or when buying this FPGA type is known."
	while true; do
	read -p "Install CL016 or CL025 radioberry version: 1 = CL016 or 2 = CL025? " type
	case $type in
		[1]* ) sudo wget https://raw.githubusercontent.com/pa3gsb/Radioberry-2.x/master/SBC/rpi-4/releases/dev/CL016/radioberry.rbf -O /lib/firmware/radioberry.rbf; break;;
		[2]* ) sudo wget https://raw.githubusercontent.com/pa3gsb/Radioberry-2.x/master/SBC/rpi-4/releases/dev/CL025/radioberry.rbf -O /lib/firmware/radioberry.rbf; break;;
		* ) echo "Please answer 1 or 2 for the FPGA used in your radioberry.";
	esac
	done
fi


# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Docker is not installed."
	echo "Installeer Docker and try again: . curl -sSL https://get.docker.com | sh"
    exit 1
fi

echo "Docker is installed." 
sudo docker -v

sudo docker run --rm \
    -v /usr/src:/usr/src \
    -v /lib/modules/$(uname -r):/lib/modules/$(uname -r) \
    -v /lib:/lib \
	-v /tmp/radioberry_kernel_pi4:/tmp/radioberry_kernel_pi4 \
    pa3gsb/radioberry_kernel_pi4


# Controleer de exit-code van de Docker-container
if [ $? -eq 0 ]; then
    echo "Docker container finished successfully, continuing..."
else
    echo "Building radioberry driver failed (using Docker), stopping script."
    exit 1
fi

# stop running services
sudo systemctl stop radioberry > /dev/null 2>&1
sudo systemctl disable radioberry > /dev/null 2>&1

# remove radioberry driver. 
sudo modprobe -r radioberry 2>&1

# Determine BOOT_PATH
if [ -d "/boot/firmware" ]; then
    BOOT_PATH="/boot/firmware"
else
    BOOT_PATH="/boot"
fi

# Create target map if not existing already
if [ ! -d "/lib/modules/$(uname -r)/kernel/drivers/sdr" ]; then
    sudo mkdir -p /lib/modules/$(uname -r)/kernel/drivers/sdr
fi

# Copy  .ko file to kernel modules directory
sudo cp /tmp/radioberry_kernel_pi4/*.ko /lib/modules/$(uname -r)/kernel/drivers/sdr

# Copy .dtbo file to the overlays directory
sudo cp /tmp/radioberry_kernel_pi4/*.dtbo "$BOOT_PATH/overlays"

# Add the driver in the config.txt if not already added.
sudo grep -Fxq "dtoverlay=radioberry" "$BOOT_PATH/config.txt" || sudo sed -i '$ a dtoverlay=radioberry' "$BOOT_PATH/config.txt"

# Update module dependencies
sudo depmod

# Load the radioberry module
sudo modprobe radioberry

# Set the permission /dev/radioberry
sudo chmod 666 /dev/radioberry

# Show radioberry module information.
sudo modinfo radioberry

# start running services
sudo systemctl enable radioberry > /dev/null 2>&1
sudo systemctl start radioberry > /dev/null 2>&1


echo ""
echo "Radioberry driver installed."

exit 0