#!/bin/bash

echo ""
echo ""
echo "============================================"
echo "Radioberry software installation."
echo ""
echo "You will install the following versions: "
echo ""
echo "	Gateware version 73.3"
echo "	Driver version 0.94"
echo "	Firmware version 2022.01.24"
echo "============================================"
echo ""
echo ""

fpgatype=2; #default
while true; do
read -p "Install CL016 or CL025 radioberry version: 1 = CL016 or 2 = CL025? " type
case $type in
	[1]* ) fpgatype=1; break;;
	[2]* ) fpgatype=2; break;;
	* ) echo "Please answer 1 or 2 for the FPGA used in your radioberry.";
esac
done

function install_dependency {
	echo "--- Installing dependency: $1"
	sudo apt-get -y install $1
}

install_dependency raspberrypi-kernel-headers
install_dependency linux-headers-rpi
install_dependency git
install_dependency device-tree-compiler
install_dependency pigpio

if [ -d "Radioberry-2.x" ]; then
    DO_CLEANUP=false
else
    DO_CLEANUP=true
	git clone  --depth=1 https://github.com/pa3gsb/Radioberry-2.x
fi

sudo systemctl stop radioberry
sudo systemctl disable radioberry

#-----------------------------------------------------------------------------

if [[ $fpgatype == 1 ]]; then
	echo "Installing Radioberry gateware Cyclone 10 CL016..."
		
cd Radioberry-2.x/SBC/rpi-4/releases/dev/CL016
sudo cp ./radioberry.rbf /lib/firmware
cd ../../../../../..
	
echo ""
echo "Radioberry gateware Cyclone 10 CL016 installed."

fi

if [[ $fpgatype == 2 ]]; then
	echo "Installing Radioberry gateware Cyclone 10 CL025..."
	
cd Radioberry-2.x/SBC/rpi-4/releases/dev/CL025
sudo cp ./radioberry.rbf /lib/firmware
cd ../../../../../..
	
echo ""
echo "Radioberry gateware Cyclone 10 CL025 installed."
	
fi

#-----------------------------------------------------------------------------
echo "Installing Radioberry driver..."

#unregister radioberry driver
sudo modprobe -r radioberry

# new raspios uses /boot/firmware instead of /boot
if [ -d "/boot/firmware" ]; then
    BOOT_PATH="/boot/firmware"
else
    BOOT_PATH="/boot"
fi
	
if [ ! -d "/lib/modules/$(uname -r)/kernel/drivers/sdr" ]; then
	sudo mkdir /lib/modules/$(uname -r)/kernel/drivers/sdr
fi
	
cd Radioberry-2.x/SBC/rpi-4/device_driver/driver
make
if [ $? -eq 0 ]; then
	sudo cp radioberry.ko /lib/modules/$(uname -r)/kernel/drivers/sdr

	sudo dtc -@ -I dts -O dtb -o radioberry.dtbo radioberry.dts
	sudo cp radioberry.dtbo "$BOOT_PATH/overlays"
	#add driver to config.txt
	sudo grep -Fxq "dtoverlay=radioberry" "$BOOT_PATH/config.txt" || sudo sed -i '$ a dtoverlay=radioberry' "$BOOT_PATH/config.txt"

	cd ../../../../..
		
	sudo depmod	
	#register radioberry driver
	sudo modprobe radioberry
	sudo chmod 666 /dev/radioberry
	#show radioberry driver info.
	sudo modinfo radioberry

	echo ""
	echo "Radioberry driver installed."
else
    echo "Radioberry driver installation failed."
    echo ""
	echo "You are using a linux version without linux header files; choose an other distro."
	
	if [ "$DO_CLEANUP" = true ]; then
		echo "Cleaning up..."
		sudo rm -rf Radioberry-2.x
	fi
	
	exit 1
fi


#-----------------------------------------------------------------------------
echo "Installing Radioberry firmware..."
	
cd Radioberry-2.x/SBC/rpi-4/device_driver/firmware
sudo make
	if [ $? -eq 0 ]; then

	sudo cp radioberry  /usr/local/bin
	sudo chmod +x /usr/local/bin/radioberry

	cd ../../../../..

	echo ""
	echo "Radioberry firmware installed."	
else
    echo "Radioberry firmware installation failed."
    echo ""
	echo "Looking into the log and try to find out what is wrong."
	
	if [ "$DO_CLEANUP" = true ]; then
		echo "Cleaning up..."
		sudo rm -rf Radioberry-2.x
	fi
	
	exit 1
fi

#-----------------------------------------------------------------------------
echo "Installing radioberry service ..."

cd Radioberry-2.x/SBC/rpi-4/device_driver/systemd

sudo cp radioberry.service /etc/systemd/system/radioberry.service
sudo cp radioberryd /etc/init.d/radioberryd
sudo chmod +x /etc/init.d/radioberryd

cd ../../../../..

echo "Radioberry service installed."

#-----------------------------------------------------------------------------
if [ "$DO_CLEANUP" = true ]; then
	echo "Cleaning up..."
	sudo rm -rf Radioberry-2.x
fi

sudo systemctl enable radioberry
sudo systemctl start radioberry


echo ""
echo ""
echo "============================================"
echo "Radioberry software is installed!"
echo "Have fun using the SDR Radioberry"
echo ""
echo "73 Johan PA3GSB"
echo "============================================"
echo ""
echo ""

exit 0
