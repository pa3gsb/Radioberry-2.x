Hermes-Lite 2.x Gateware
========================

## 20190223 Release

* Fixed small DC spur in new NCO.
* Enabled PureSignal for 4 RX architecture. The TX feedback is now on receiver 4. PowerSDR users may need to select a model other than Anan10E.
* Added Run and Idle modes for the LEDs. Please see [Getting Started Wiki Page](https://github.com/softerhardware/Hermes-Lite2/wiki/Hermes-Lite-2.0-Getting-Started#leds) for more details.
* Added .svf files for programming with urjtag or openocd. Because the FPGA establishes a bridge between JTAG and the EEPROM, the .svf files are only for volatile programming. To program the eeprom you must still use the .jic files with Quartus or the .jam files with a JAM player such as the one included in the RaspberryPi test image.


### File Key

* hl2b2 - Hermes-Lite 2.0 beta2
* hl2b3to4 - Hermes-Lite 2.0 beta3 or beta4
* hl2b5up - Hermes-Lite 2.0 build5 and later
* hl2b5upce15 - Hermes-Lite 2.0 build5 and later when using smaller EP4CE15E22C8N FPGA. Currently supports only 1 receiver.


* .jic - Nonvolatile EEPROM programming with Quartus
* .sof - Volatile FPGA-only programming with Quartus
* jic.jam - Nonvolatile EEPROM programming with JAM/STAPL player
* sof.jam - Volatile FPGA-only programming with JAM/STAPL player
* .svf - Volatile FPGA-only programming with urjtag or openocd 





