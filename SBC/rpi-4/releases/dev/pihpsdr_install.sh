#!/bin/bash
echo "Installation wdsp or pihpsdr"
echo "----------------------------"
echo "NOTE. Before building and installing pihpsdr you should to install wdsp."
echo ""

echo "What do you like to install [wdsp/pihpsdr/reboot] ?"

read input

if [[ $input == "wdsp" ]]; then
	echo ""
	echo "Installing WDSP library..."

	function install_dependency {
		echo "--- Installing dependency: $1"
		sudo apt-get -y install $1
	}

	install_dependency git
	install_dependency libfftw3-dev
	
	cd /tmp
	git clone --depth=1 https://github.com/g0orx/wdsp
	cd wdsp
	make -j 4
	sudo make install
	cd ..
	rm -rf wdsp
	cd ~

	echo ""
	echo "WDSP installed."

elif [[ $input == "pihpsdr" ]]; then
	
	echo ""
	echo "Installing pihpsdr..."
	echo ""
	echo ""
	localcw=0;
	while true; do
    read -p "Do you wish to install the local CW option: yes or no?" yn
    case $yn in
        [Yy]* ) localcw=1; break;;
        [Nn]* ) break;;
        * ) echo "Please answer yes (Y or y) or no (N or n).";;
    esac
	done
	
	function install_dependency {
		echo "--- Installing dependency: $1"
		sudo apt-get -y install $1
	}

	install_dependency git
	install_dependency libpulse-dev
	install_dependency libgtk-3-dev
	install_dependency libasound2-dev
	install_dependency libcurl4-openssl-dev
	install_dependency libusb-1.0-0-dev
	install_dependency libgpiod-dev
	install_dependency gpiod
	install_dependency libi2c-dev
	
	cd /tmp
	git clone https://github.com/g0orx/pihpsdr.git
	cd pihpsdr
	
	#makefile modification; when 'on' switch it 'off' (adding '#'). 
	sed -i '/^PURESIGNAL_INCLUDE=PURESIGNAL/c\#PURESIGNAL_INCLUDE=PURESIGNAL' ./Makefile;
	sed -i '/^#MIDI_INCLUDE=MIDI/c\MIDI_INCLUDE=MIDI' ./Makefile;
	sed -i '/^GPIO_INCLUDE=GPIO/c\#GPIO_INCLUDE=GPIO' ./Makefile;
	sed -i '/^LOCALCW_INCLUDE=LOCALCW/c\#LOCALCW_INCLUDE=LOCALCW' ./Makefile;
		
	if [ "$localcw" -eq "1" ]; then
		sed -i '/^#GPIO_INCLUDE=GPIO/c\GPIO_INCLUDE=GPIO' ./Makefile;
		sed -i '/^#LOCALCW_INCLUDE=LOCALCW/c\LOCALCW_INCLUDE=LOCALCW' ./Makefile;
	fi
	

	make -j 4
	sudo make install
	if [ ! -d "${HOME}/.pihpsdr" ]; then
		mkdir ${HOME}/.pihpsdr
	fi
	cp release/pihpsdr/hpsdr.png ${HOME}/.pihpsdr
	cd ..
	rm -rf pihpsdr
	
	rm ${HOME}/Desktop/pihpsdr.desktop || true
	file="${HOME}/Desktop/pihpsdr.desktop"
	echo "[Desktop Entry]" >> $file
	echo "Icon=${HOME}/.pihpsdr/hpsdr.png" >> $file
	echo "Exec=/usr/local/bin/pihpsdr" >> $file
	echo "Type=Application" >> $file
	echo "Terminal=false" >> $file
	echo "Path=${HOME}/.pihpsdr" >> $file
	cat $file
	
	echo ""
	echo "pihpsdr installed"

elif [[ $input == "reboot" ]]; then	

	sudo reboot

else 
	echo "You did not make a selection; Nothing installed!"
	echo "73!"
fi
