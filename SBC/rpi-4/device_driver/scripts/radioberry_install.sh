#!/bin/bash

echo "Installation of radioberry setup"
echo "You need to install [CL016/CL025/driver/firmware/systemd/reboot] ?"

read input

if [[ $input == "CL025" ]]; then
	echo "Installing Radioberry gateware Cyclone 10 CL025..."
	
	wget https://raw.githubusercontent.com/softerhardware/Hermes-Lite2/master/gateware/variants/radioberry_cl025/build/radioberry.rbf
	sudo mv ./radioberry.rbf /lib/firmware
	
	echo ""
	echo "Radioberry gateware Cyclone 10 CL025 installed."
	
elif [[ $input == "CL016" ]]; then
	echo "Installing Radioberry gateware Cyclone 10 CL016..."
	
	wget https://raw.githubusercontent.com/softerhardware/Hermes-Lite2/master/gateware/variants/radioberry_cl016/build/radioberry.rbf
	sudo mv ./radioberry.rbf /lib/firmware
	
	echo ""
	echo "Radioberry gateware Cyclone 10 CL016 installed."

elif [[ $input == "driver" ]]; then

	echo "Installing Radioberry driver..."
	
	function install_dependency {
		echo "--- Installing dependency: $1"
		sudo apt-get -y install $1
	}

	install_dependency raspberrypi-kernel-headers
	install_dependency git
	install_dependency device-tree-compiler
	
	sudo systemctl stop radioberry
	sudo systemctl disable radioberry
	
	#unregister radioberry driver
	sudo modprobe -r radioberry
	
	if [ ! -d "/lib/modules/$(uname -r)/kernel/drivers/sdr" ]; then
		sudo mkdir /lib/modules/$(uname -r)/kernel/drivers/sdr
	fi
	
	git clone  --depth=1 https://github.com/pa3gsb/Radioberry-2.x
	cd Radioberry-2.x/SBC/rpi-4/device_driver/driver
	make
	sudo cp radioberry.ko /lib/modules/$(uname -r)/kernel/drivers/sdr
	
	sudo dtc -@ -I dts -O dtb -o radioberry.dtbo radioberry.dts
	sudo cp radioberry.dtbo /boot/overlays
	#add driver to config.txt
	sudo grep -Fxq "dtoverlay=radioberry" /boot/config.txt || sudo sed -i '$ a dtoverlay=radioberry' /boot/config.txt

	cd ../../../../..
	rm -rf Radioberry-2.x
	
	sudo depmod	
	#register radioberry driver
	sudo modprobe radioberry
	sudo chmod 666 /dev/radioberry
	#show radioberry driver info.
	sudo modinfo radioberry
	
	sudo systemctl enable radioberry
	sudo systemctl start radioberry
	
	echo ""
	echo "Radioberry driver installed."

elif [[ $input == "firmware" ]]; then	

	echo "Installing Radioberry firmware..."

	function install_dependency {
		echo "--- Installing dependency: $1"
		sudo apt-get -y install $1
	}

	install_dependency pigpio
	install_dependency git
		
	sudo systemctl stop radioberry
	sudo systemctl disable radioberry
		
	git clone  --depth=1 https://github.com/pa3gsb/Radioberry-2.x
	cd Radioberry-2.x/SBC/rpi-4/device_driver/firmware
	sudo make
	sudo cp radioberry  /usr/local/bin
	sudo chmod +x /usr/local/bin/radioberry
	
	cd ../../../../..
	rm -rf Radioberry-2.x
	
	sudo systemctl enable radioberry
	sudo systemctl start radioberry
	
	echo ""
	echo "Radioberry firmware installed."	

elif [[ $input == "systemd" ]]; then	

	echo "Installing automatic radioberry firmware start ..."

	function install_dependency {
		echo "--- Installing dependency: $1"
		sudo apt-get -y install $1
	}

	install_dependency raspberrypi-kernel-headers
	install_dependency git
		
	sudo systemctl stop radioberry
	sudo systemctl disable radioberry
		
	git clone --depth=1 https://github.com/pa3gsb/Radioberry-2.x
	cd Radioberry-2.x/SBC/rpi-4/device_driver/systemd
	
	sudo cp radioberry.service /etc/systemd/system/radioberry.service
	sudo cp radioberryd /etc/init.d/radioberryd
	sudo chmod +x /etc/init.d/radioberryd
	
	cd ../../../../..
	rm -rf Radioberry-2.x
	
	sudo systemctl enable radioberry
	sudo systemctl start radioberry
	
	echo ""
	echo "Automatic starting Radioberry installed."
	
elif [[ $input == "reboot" ]]; then	

	sudo reboot

else 
	echo "You did not make a selection; Nothing installed!"
	echo "73!"
fi
