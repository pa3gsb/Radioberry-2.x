#!/bin/bash

echo "Installation of radioberry SOAPY SDR setup"
echo ""
echo "You need to install [SoapySDR/SoapyRadioberrySDR/reboot] ?"

read input

if [[ $input == "SoapySDR" ]]; then
	echo "Installing SoapySDR..."
	
	function install_dependency {
		echo "--- Installing dependency: $1"
		sudo apt-get -y install $1
	}

	install_dependency git
	install_dependency cmake
	install_dependency libusb-1.0-0-dev
	
	cd /tmp
	git clone https://github.com/pothosware/SoapySDR.git
	cd SoapySDR

	mkdir build
	cd build
	cmake ..
	make -j4
	sudo make install
	sudo ldconfig
	cd ../..
	rm -rf SoapySDR
	
	SoapySDRUtil --info
	
	echo ""
	echo "SoapySDR  installed."
	
elif [[ $input == "SoapyRadioberrySDR" ]]; then
	echo "Installing SoapyRadioberrySDR ..."
	
	function install_dependency {
		echo "--- Installing dependency: $1"
		sudo apt-get -y install $1
	}

	install_dependency git
	install_dependency cmake
	
	cd /tmp
	git clone --depth=1 https://github.com/pa3gsb/Radioberry-2.x.git
	cd Radioberry-2.x/SBC/rpi-4/SoapyRadioberrySDR
	mkdir build
	cd build
	cmake ..
	make -j4
	sudo make install
	sudo ldconfig
	cd ../..
	rm -rf Radioberry-2.x

	SoapySDRUtil --probe="driver=radioberry"

	echo ""
	echo "SoapyRadioberrySDR installed."

elif [[ $input == "reboot" ]]; then	

	sudo reboot

else 
	echo "You did not make a selection; Nothing installed!"
	echo "73!"
fi


