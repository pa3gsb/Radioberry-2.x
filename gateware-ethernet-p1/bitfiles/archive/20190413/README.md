Hermes-Lite 2.x Gateware
========================

## 20190413 Release

This release adds support for gateware update over ethernet. This has been tested with the [HPSDRProgrammerV2-nopcap](https://github.com/TAPR/OpenHPSDR-Protocol1-Programmers/releases) programmer. Update to this 20190413 release via JTAG using the .jic (nonvolatile) file, or the .sof (volatile) file. If using the .sof file, immediately update using the HPSDRProgrammer with the .rbf file so that your update is nonvolatile. If you want to test this new feature, you can then update to the 20190412 release and see the software version increment from 64 to 65.

The Hermes-Lite currently just has a single gateware image stored in the EEPROM. This image is overwritten with every update. There is not yet a factory/appication dual image setup.


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





