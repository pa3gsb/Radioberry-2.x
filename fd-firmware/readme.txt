Experiments to use the Radioberry in full duplex mode.

(!!!!!Only CL025 FPGA!!!!!!)

To increase speed of reading the RX IQ data transfer from FPGA to the raspberry pi (rpi) a DDR (Double Data Rate) algorithm is introduced.
This algorithm is providing the data to the rpi in parallell mode iso of using the serial transfer using SPI.

The parallell read is done in the user space of the operating system, costing some performance but makes larger sample rates possible.

This mod is resulting in 2 RX slices with a sample rate of 192K.

The radioberry firmware is formed by the radioberry.rbf (verilog) and the radioberry (c-language) executable running at a rpi.
Radioberry.rbf firmware must be loaded into the FPGA by using the loadFPGA utility. 


Big advantage of full duplex is the enabling of Puresignal and VNA.

I also like to include an EER!

Hardware Modification:
To make use of FIR filters which are working on double clock, reducing some FPGA resources a small hardware mod is required.

An additonal clock on pin 53 of the FPGA must be provided by connecting this to pin 69 of the FPGA (connected to the CLKOUT1 of the AD9866).
Using this clock a FPGA PLL circuit can be used to produce the right clock!

Puresignal
xxxxxxxxxxxxxxxx

VNA
Not looked into yet....

EER
yyyyyyyyyyyyyy



