set_time_format -unit ns -decimal_places 3

create_clock -name ad9866_clk  	-period 76.800MHz	[get_ports ad9866_clk] 
create_clock -name ad9866_rxclk -period 76.800MHz	[get_ports ad9866_rxclk] 
create_clock -name ad9866_txclk -period 76.800MHz  [get_ports ad9866_txclk] 
create_clock -name clk_10mhz -period 10.000MHz [get_ports clk_10mhz] 
create_clock -name spi_sck -period 15.625MHz [get_ports spi_sck] 
create_clock -name spi_ce0 -period 0.400MHz [get_ports {spi_ce[0]}]
create_clock -name spi_ce1 -period 0.400MHz [get_ports {spi_ce[1]}]

create_clock -name {spi_slave:spi_slave_rx2_inst|done} -period 0.400MHz [get_registers {spi_slave:spi_slave_rx2_inst|done}]
create_clock -name {spi_slave:spi_slave_rx_inst|done} -period 0.400MHz [get_registers {spi_slave:spi_slave_rx_inst|done}]

create_clock -name virt_ad9866_clk -period 76.800MHz
create_clock -name virt_ad9866_rxclk -period 76.800MHz
create_clock -name virt_ad9866_txclk -period 76.800MHz
create_clock -name {ad9866:ad9866_inst|dut1_pc[0]} -period 10.000 [get_registers {ad9866:ad9866_inst|dut1_pc[0]}]
create_clock -name {PLL_IAMBIC_inst|altpll_component|auto_generated|pll1|clk[0]} -period 0.19MHz
create_clock -name {PLL_IAMBIC_inst|altpll_component|auto_generated|pll1|clk[1]} -period 0.03MHz

derive_pll_clocks

derive_clock_uncertainty

set_clock_groups -asynchronous \
						-group {ad9866_clk ad9866_rxclk ad9866_txclk} \
						-group {	clk_10mhz } \
						-group {	spi_ce0 } \
						-group {	spi_ce1 } \
						-group {	spi_slave:spi_slave_rx_inst|done } \
						-group {	spi_slave:spi_slave_rx2_inst|done } \
						-group { ad9866:ad9866_inst|dut1_pc[0]} \
						-group { PLL_IAMBIC_inst|altpll_component|auto_generated|pll1|clk[0]} \
						-group { PLL_IAMBIC_inst|altpll_component|auto_generated|pll1|clk[1]}
				
# CLOCK					
set_false_path -from [get_ports {clk_10mhz}]		

# IO
set_input_delay -add_delay -max -clock spi_sck 1.0 [get_ports {spi_mosi}]
set_input_delay -add_delay -min -clock spi_sck -1.0 [get_ports {spi_mosi}]
set_output_delay -add_delay -max -clock spi_sck 1.0 [get_ports {spi_miso}]
set_output_delay -add_delay -min -clock spi_sck -1.0 [get_ports {spi_miso}]
set_input_delay -add_delay -max -clock spi_sck 1.0 [get_ports {spi_ce[*]}]
set_input_delay -add_delay -min -clock spi_sck -1.0 [get_ports {spi_ce[*]}]

set_false_path -to [get_ports {rx1_samples}]
set_false_path -to [get_ports {rx2_samples}]
set_false_path -to [get_ports {pistrobe}]
set_false_path -to [get_ports {txFIFOFull}]

set_false_path -from * -to { key_dot_rpi key_dash_rpi cw_ptt rb_info_1 rb_info_2 ptt_out filter[*]}
set_false_path -from {ptt_in KEY_DOT KEY_DASH} -to *


## AD9866 RX Path
## See http://billauer.co.il/blog/2017/04/altera-intel-fpga-io-ff-packing/

set_output_delay -add_delay -max -clock virt_ad9866_rxclk 1.0 [get_ports {ad9866_rxen}]
set_output_delay -add_delay -min -clock virt_ad9866_rxclk -1.0 [get_ports {ad9866_rxen}]

set_input_delay -add_delay -max -clock virt_ad9866_rxclk 10.0 [get_ports {ad9866_adio[*]}]
set_input_delay -add_delay -min -clock virt_ad9866_rxclk 4.5 [get_ports {ad9866_adio[*]}]



## AD9866 TX Path

set_output_delay -add_delay -max -clock virt_ad9866_txclk 1.4 [get_ports {ad9866_txen}]
set_output_delay -add_delay -min -clock virt_ad9866_txclk -1.3 [get_ports {ad9866_txen}]

set_output_delay -add_delay -max -clock virt_ad9866_txclk 1.4 [get_ports {ad9866_adio[*]}]
set_output_delay -add_delay -min -clock virt_ad9866_txclk -1.3 [get_ports {ad9866_adio[*]}]


## AD9866 Other IO
set_false_path -to [get_ports {ad9866_sclk}]
set_false_path -to [get_ports {ad9866_sdio}]
set_false_path -from [get_ports {ad9866_sdo}]
set_false_path -to [get_ports {ad9866_sen_n}]
set_false_path -to [get_ports {ad9866_rst_n}]
set_false_path -to [get_ports {ad9866_mode}]


## Additional timing constraints
set_max_delay -from spi_ce[*] -to spi_miso 10
set_max_delay -from spi_ce[0]	-to spi_slave:spi_slave_rx_inst|treg[*] 5
set_min_delay -from spi_ce[0]	-to spi_slave:spi_slave_rx_inst|rreg[*] -3
set_max_delay -from spi_ce[1]	-to spi_slave:spi_slave_rx2_inst|treg[*] 3
set_min_delay -from spi_ce[1]	-to spi_slave:spi_slave_rx2_inst|rreg[*] -3

set_max_delay -from transmitter:transmitter_inst|out_data[*]	-to ad9866_adio[*] 18
						  
set_max_delay -from rxFIFO:rx1_FIFO_inst|dcfifo:dcfifo_component|dcfifo_nkk1:auto_generated|altsyncram_b271:fifo_ram|q_b[*]	-to spi_slave:spi_slave_rx_inst|treg[*] 4
set_max_delay -from rxFIFO:rx2_FIFO_inst|dcfifo:dcfifo_component|dcfifo_nkk1:auto_generated|altsyncram_b271:fifo_ram|q_b[*]	-to spi_slave:spi_slave_rx2_inst|treg[*] 4

set_max_delay -from rxLargeFIFO:rx1_FIFO_inst|dcfifo:dcfifo_component|dcfifo_0lk1:auto_generated|altsyncram_d271:fifo_ram|q_b[*]	-to spi_slave:spi_slave_rx_inst|treg[*] 4
						 
set_max_delay -from spi_slave:spi_slave_rx_inst|rdata[*]	-to txFIFO:txFIFO_inst|dcfifo:dcfifo_component|dcfifo_ngk1:auto_generated|altsyncram_v171:fifo_ram|ram_block9a0~porta_datain_reg0 4
						 
set_max_delay -from  reset_handler:reset_handler_inst|reset~_Duplicate_1	-to spi_slave:spi_slave_rx_inst|rdata[*] 5
set_max_delay -from  reset_handler:reset_handler_inst|reset~_Duplicate_1	-to spi_slave:spi_slave_rx2_inst|treg[*] 5

set_min_delay -from spi_mosi	-to spi_slave:spi_slave_rx_inst|rdata[*] -2
	
## end of constraints