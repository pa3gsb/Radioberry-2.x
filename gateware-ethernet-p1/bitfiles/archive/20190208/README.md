Hermes-Lite 2.x Firmware
========================

20190208 Release

This release is to test new numerically controlled oscillators as a replacement for the current Cordic. Simulation results show that the NCO output is cleaner than the Cordic. The NCO requires about 1/10th of the LUTs as the Cordic, but does use 2 multipliers and 1/4 of a RAM block per oscillator. Because of the overall savings in resources, there are 4 receivers in this firmware. Eventually, after some work on the other filters, we should be able to have >5 receivers.

I'd appreciate people checking for spurs, anomalies or other shortcomings with these new oscaillators. To help comparison, the main receiver 1 is still the original, receivers 2 and 3 are one variation of the new NCO, and receiver 4 is another slight variation of the new NCO.

