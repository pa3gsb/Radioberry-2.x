
Hereby the latest release (release-27052018.tar) for the radioberry 2.0 beta 2.


copy the file to home folder eg /home/pi

extract the tar file: command: tar xvf release-27052018.tar

change to folder : release-27052018

using command sudo ./launch.sh 

this commands set the clock, loads the firmware into the FPGA and starts the hermes emulator.

Now it is possible to start your favourite SDR program. (assuming your rpi is connected to the network)


With wdsp and libfftw3 already installed it is also possible to run pihpdsr.

In the release folder also the pihpsdr program is present.

Open a different terminal and start the program: sudo ./pihpsdr

The first time start the wisdom files will be created.

Goodluck with it!

Johan PA3GSB

