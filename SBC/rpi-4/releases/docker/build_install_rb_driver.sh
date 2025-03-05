#!/bin/bash


# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Docker is not installedtalleerd."
	echo "Installeer Docker and try again: . curl -sSL https://get.docker.com | sh"
    exit 1
fi

echo "Docker is installed." 
sudo docker -v

sudo docker run --rm \
    -v /usr/src:/usr/src \
    -v /lib/modules/$(uname -r):/lib/modules/$(uname -r) \
    -v /lib:/lib \
	-v /tmp/radioberry_kernel:/tmp/radioberry_kernel \
    pa3gsb/radioberry_kernel_pi4


# Controleer de exit-code van de Docker-container
if [ $? -eq 0 ]; then
    echo "Docker container finished successfully, continuing..."
else
    echo "Building radioberry driver failed (using Docker), stopping script."
    exit 1
fi

# stop running services
sudo systemctl stop radioberry 2>&1
sudo systemctl disable radioberry 2>&1

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

echo ""
echo "Radioberry driver installed."

# start running services
sudo systemctl enable radioberry > /dev/null 2>&1
sudo systemctl start radioberry > /dev/null 2>&1


exit 0