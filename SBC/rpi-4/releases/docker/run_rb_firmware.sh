#!/bin/bash

echo ""
echo ""
echo "============================================"
echo ""
echo "Radioberry firmware"
echo ""
echo "============================================"
echo ""
echo ""


# stop running services
sudo systemctl stop radioberry > /dev/null 2>&1
sudo systemctl disable radioberry > /dev/null 2>&1

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Docker is not installed."
	echo "Installeer Docker and try again: . curl -sSL https://get.docker.com | sh"
    exit 1
fi

echo "Docker is installed." 
sudo docker -v


if [ -e /dev/radioberry ]; then
    echo "/dev/radioberry present, start Radioberry firmware by using Docker-container..."
    sudo docker run --rm \
        --device /dev/radioberry \
        --device /dev/i2c-1 \
        --net host \
        -v /home/pi/.radioberry:/home/pi/.radioberry \
        pa3gsb/radioberry_firmware
else
    echo "ERROR: /dev/radioberry not found! Please run first build_install_rb_driver.sh."
    exit 1
fi

# start running services
sudo systemctl enable radioberry > /dev/null 2>&1
sudo systemctl start radioberry > /dev/null 2>&1

echo ""
echo "Nice you are using Radioberry hope you enjoyed it!"
echo ""
echo "if you are facing issues or having ideas for improvement use the forum."
echo "The forum can be found at https://groups.google.com/g/radioberry"
echo ""
echo "Have fun"
echo "Johan, PA3GSB"