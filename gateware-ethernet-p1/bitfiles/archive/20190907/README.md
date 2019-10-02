Hermes-Lite 2.x Gateware
========================

## 20190907 Release

 This version identifies as revision 67. See the [wiki](https://github.com/softerhardware/Hermes-Lite2/wiki/Updating-Gateware) for instructions on how to update your HL2 gateware. Changes in this version are listed below.

 * Some improvements to TX sequencing. There is now a proper TX hang that ensures all data sent as TX is sent to the DAC. The levels of the TX buffer are reset after each TX. This ensures that software will see a constant delay of about 22ms from TX data sent to TX signal on the antenna. The HL2 buffer can handle network or host jitter of about +/-20ms.

 * Primary RX oscillator switched from Cordic to NCO. Recent gateware has been testing a new NCO for RX1,RX2 and RX3. This new NCO has worked well and saves on FPGA resources. The primary RX0 is now using the same NCO in preparation for future changes which require more FPGA resources.

 * FWD and REV power are swapped. The data sent back to software is now swapped to better match PowerSDR. Quisk and SparkSDR autodetect which values are FWD and REV and so are not affected by this change.

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





