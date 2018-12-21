
The folder wsprberry-verilog-fpga contains the latest firmware for using wspr.

For the CL025 now it is possible to receive 5 channels
For the CL016 now it is possible to receive 3 channels

Pay attention :

HARDWARE MODIFICATION:
To make use of the new FIR filters which are working on double clock, reducing some FPGA resources a small hardware mod is required.

An additonal clock on pin 53 of the FPGA must be provided by connecting this to pin 69 of the FPGA (connected to the CLKOUT1 of the AD9866).
Using this clock a FPGA PLL circuit can be used to produce the right clock!
(required for the beta 2, 3 PCB, i will include the change in beta 4)