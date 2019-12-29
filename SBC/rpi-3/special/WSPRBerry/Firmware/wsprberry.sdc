set_time_format -unit ns -decimal_places 3

create_clock -name ad9866_clk  	-period 73.728MHz	[get_ports ad9866_clk] 
create_clock -name ad9866_rxclk -period 73.728MHz	[get_ports ad9866_rxclk] 
create_clock -name ad9866_txclk -period 73.728MHz  [get_ports ad9866_txclk] 
create_clock -name clk_10mhz -period 10.000MHz [get_ports clk_10mhz] 
create_clock -name spi_sck -period 144KHz [get_ports spi_sck] 
create_clock -name spi_ce0 -period 5100Hz [get_ports {spi_ce[0]}]

create_clock -name virt_ad9866_clk -period 73.728MHz
create_clock -name virt_ad9866_rxclk -period 73.728MHz

derive_pll_clocks

derive_clock_uncertainty

set_clock_groups -asynchronous \
						-group {ad9866_clk ad9866_rxclk ad9866_txclk} \
						-group {	clk_10mhz } \
						-group {	spi_ce0 } 
				
#*************************************************************************************************************
# set input delay
#*************************************************************************************************************
set_input_delay -add_delay -max -clock ad9866_clk 1.000 {ad9866_sdo spi_sck spi_mosi}
set_input_delay -add_delay -min -clock ad9866_clk -1.000 {ad9866_sdo spi_sck spi_mosi}

set_input_delay -add_delay -max -clock virt_ad9866_clk 10.000 [get_ports {ad9866_adio[*]}]
set_input_delay -add_delay -min -clock virt_ad9866_clk 0.500 [get_ports {ad9866_adio[*]}]

#*************************************************************************************************************
# set output delay
#*************************************************************************************************************
set_output_delay 1.000 -clock ad9866_clk {spi_miso rx_FIFOEmpty ad9866_rxclk ad9866_txclk ad9866_sclk ad9866_sdio}
set_output_delay  -add_delay -max -clock virt_ad9866_rxclk  5.000 {ad9866_adio[*]}
set_output_delay  -add_delay -min -clock virt_ad9866_rxclk  0.500 {ad9866_adio[*]}

set_max_delay -from spi_slave:spi_slave_rx_inst|treg[*]  -to spi_miso 15
set_max_delay -from spi_ce[0]	-to spi_slave:spi_slave_rx_inst|treg[*] 3

set_max_delay -from nnrx[*]	-to spi_slave:spi_slave_rx_inst|treg[*] 7

set_max_delay -from spi_mosi	-to spi_slave:spi_slave_rx_inst|rdata[0] 8
set_max_delay -from spi_mosi 	-to spi_slave:spi_slave_rx_inst|rreg[0] 8

set_max_delay -from ad9866_adio[*]	-to adcpipe[*][*] 14

set_max_delay -from spi_slave:spi_slave_rx_inst|rdata[*] -to rx_freq[*][*] 4
set_max_delay -from spi_slave:spi_slave_rx_inst|rdata[*] -to nnrx[*] 4
set_max_delay -from spi_slave:spi_slave_rx_inst|rdata[*] -to att[*] 4
set_max_delay -from spi_slave:spi_slave_rx_inst|rdata[*]	-to dither 3
set_max_delay -from spi_ce[0] -to spi_slave:spi_slave_rx_inst|done 3
set_max_delay -from spi_slave:spi_slave_rx_inst|rdata[*] -to	spi_ce0 4
set_max_delay -from spi_slave:spi_slave_rx_inst|rreg[*]	-to spi_ce0 3
set_max_delay -from spi_ce[0]	-to spi_slave:spi_slave_rx_inst|rdata[*] 3
set_max_delay -from spi_ce[0]	-to spi_slave:spi_slave_rx_inst|rdata[*] 3
set_max_delay -from spi_ce[0]	-to spi_slave:spi_slave_rx_inst|rreg[*] 5
set_max_delay -from spi_slave:spi_slave_rx_inst|nb[*] -to spi_ce0 3
set_max_delay -from spi_ce[0] -to spi_slave:spi_slave_rx_inst|nb[*] 3

set_max_delay -from rxFIFO:NRX[*].rxFIFO_inst|dcfifo:dcfifo_component|dcfifo_2pj1:auto_generated|wrptr_g[*]	-to rx_FIFOEmpty 16
set_max_delay -from rxFIFO:NRX[*].rxFIFO_inst|dcfifo:dcfifo_component|dcfifo_2pj1:auto_generated|ws_dgrp_reg[*] -to rx_FIFOEmpty 16
set_max_delay -from rxFIFO:NRX[*].rxFIFO_inst|dcfifo:dcfifo_component|dcfifo_2pj1:auto_generated|altsyncram_ti31:fifo_ram|q_b[*]	-to spi_slave:spi_slave_rx_inst|treg[*] 6

#*************************************************************************************************************
# Set False Path
#*************************************************************************************************************
# don't need fast paths to the LEDs and adhoc outputs so set false paths so Timing will be ignored
set_false_path -from * -to { DEBUG_LED*  ad9866_mode ad9866_pga[*] ad9866_rst_n ad9866_sen_n }

#don't need fast paths from the following inputs
set_false_path -from reset_handler:reset_handler_inst|reset -to *


