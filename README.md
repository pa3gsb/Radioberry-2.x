RadioBerry V2.0 - Ham radio cape for Raspberry PI
==============================================

## Main purpose of the project:

- Building a HAM Radio
- Learning (from noob to guru)

WIKI FOR MORE DETAILS:  https://github.com/pa3gsb/Radioberry-2.x/wiki

## Radioberry:

- Raspberry PI
- Radio extension board (cape)
	- using AD9866 (12 bit)  for RX and TX modes.

	![Radioberry-2.x](docs/gallery/front.JPG)
	![Radioberry-2.x](docs/gallery/radioberry-in-case-small.jpg)
	![Radioberry-2.x](docs/gallery/back.JPG)
	![Radioberry-2.x](docs/gallery/rb+rpi-front.JPG)
	![Radioberry-2.x](docs/gallery/rb+rpi-side.JPG)
	![Radioberry-2.x](docs/gallery/radioberry-open-incase-small.jpg)
	

## Building

### Install dependencies
	sudo apt-get update
	sudo apt-get install git libpulse-dev libgtk-3-dev libasound2-dev libcurl4-openssl-dev \
		libi2c-dev libgpiod-dev libusb-1.0-0-dev raspberrypi-kernel-headers device-tree-compiler pigpio

### Obtain the source codes
	cd /tmp
	git clone --depth=1 https://github.com/pa3gsb/Radioberry-2.x.git
	cd Radioberry-2.x
	git submodule update --init --recursive --depth=1

### Build the software
	make -j$(nproc)

## Installation
	sudo make install FPGATYPE=CL016
