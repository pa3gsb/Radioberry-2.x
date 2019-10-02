Hermes-Lite 2.x Gateware
========================

## 20190616 Release

This release adds support to generate the power supply switching clock directly from the FPGA. This is to move spurs outside of any amateur radio bands. This feature can be disabled and internal clock used for the power supply regulators if the ADC "random" setting is turned on.

To enable this feature, remove the 0 Ohm jumpers J2 and J11 on the HL2. Install these 0 Ohm jumpers at R13 and R16. If available, install 10K resistors at J2 and J11. The 10K resistors are not required.

 

### File Key

* hl2b2 - Hermes-Lite 2.0 beta2
* hl2b3to4 - Hermes-Lite 2.0 beta3 or beta4
* hl2b5up - Hermes-Lite 2.0 build5 and later
* hl2b5upce15 - Hermes-Lite 2.0 build5 and later when using smaller EP4CE15E22C8N FPGA. Currently supports only 1 receiver.


* .jic - Nonvolatile EEPROM programming with Quartus
* .rbf - Raw binary format for programming over ethernet using openhpsdr protocol 1
* .sof - Volatile FPGA-only programming with Quartus
* jic.jam - Nonvolatile EEPROM programming with JAM/STAPL player
* sof.jam - Volatile FPGA-only programming with JAM/STAPL player
* .svf - Volatile FPGA-only programming with urjtag or openocd 





