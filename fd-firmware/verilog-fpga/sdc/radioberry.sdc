set_time_format -unit ns -decimal_places 3

create_clock -name spi_sck -period 15.625MHz [get_ports spi_sck] 
create_clock -name spi_ce0 -period 0.400MHz [get_ports {spi_ce[0]}]
create_clock -name spi_ce1 -period 0.400MHz [get_ports {spi_ce[1]}]
create_clock -name pi_clk -period 4.800MHz [get_ports pi_clk] 
create_clock -name pi_clk2 -period 4.800MHz [get_ports pi_clk2] 

create_clock -name {ddr_mux:ddr_mux_inst1|rd_req} -period 0.200MHz [get_registers {ddr_mux:ddr_mux_inst1|rd_req}]
create_clock -name {ddr_mux:ddr_mux_inst2|rd_req} -period 0.200MHz [get_registers {ddr_mux:ddr_mux_inst2|rd_req}]
create_clock -name {spi_slave:spi_slave_rx2_inst|done} -period 0.400MHz [get_registers {spi_slave:spi_slave_rx2_inst|done}]
create_clock -name {spi_slave:spi_slave_rx_inst|done} -period 0.400MHz [get_registers {spi_slave:spi_slave_rx_inst|done}]

create_clock -name clk_76m8 -period 76.800MHz [get_ports clk_76m8]

create_clock -name virt_ad9866_rxclk_rx -period 153.600MHz
create_clock -name virt_ad9866_rxclk_tx -period 153.600MHz

create_clock -name {ad9866:ad9866_inst|dut1_pc[0]} -period 10.000 [get_registers {ad9866:ad9866_inst|dut1_pc[0]}]

derive_pll_clocks

derive_clock_uncertainty

set_clock_groups -asynchronous \
						-group { 	clk_76m8 }\
						-group {	ad9866pll_inst|altpll_component|auto_generated|pll1|clk[0]} \
						-group {	ad9866pll_inst|altpll_component|auto_generated|pll1|clk[1]} \
						-group {	ad9866pll_inst|altpll_component|auto_generated|pll1|clk[2]} \
						-group {	ad9866pll_inst|altpll_component|auto_generated|pll1|clk[3]} \
						-group {	pi_clk } \
						-group {	pi_clk2 } \
						-group {	spi_ce0 } \
						-group {	spi_ce1 } \
						-group {	spi_slave:spi_slave_rx_inst|done } \
						-group {	spi_slave:spi_slave_rx2_inst|done } \
						-group {	ddr_mux:ddr_mux_inst1|rd_req} \
						-group {	ddr_mux:ddr_mux_inst2|rd_req} \
						-group { 	ad9866:ad9866_inst|dut1_pc[0]}
				
# CLOCK						
set_false_path -from [get_ports {pi_clk}]	
set_false_path -from [get_ports {pi_clk2}]		
set_false_path -from {ad9866pll_inst|altpll_component|auto_generated|pll1|clk[3]}

# IO
set_false_path -from [get_ports {spi_mosi}]
set_false_path -to [get_ports {spi_miso}]
set_false_path -from [get_ports {spi_ce[*]}]
set_false_path -to [get_ports {rx_samples}]
set_false_path -to [get_ports {data[*]}]
set_false_path -from {ptt_in} -to *


## AD9866 RX Path
## See http://billauer.co.il/blog/2017/04/altera-intel-fpga-io-ff-packing/
set_input_delay -add_delay -max -clock virt_ad9866_rxclk_rx 5.0 [get_ports {ad9866_rxsync}]
set_input_delay -add_delay -min -clock virt_ad9866_rxclk_rx 0.0 [get_ports {ad9866_rxsync}]

set_input_delay -add_delay -max -clock virt_ad9866_rxclk_rx 5.0 [get_ports {ad9866_rx[*]}]
set_input_delay -add_delay -min -clock virt_ad9866_rxclk_rx 0.0 [get_ports {ad9866_rx[*]}]


## AD9866 TX Path

set_output_delay -add_delay -max -clock virt_ad9866_rxclk_tx 2.5 [get_ports {ad9866_txsync}]
set_output_delay -add_delay -min -clock virt_ad9866_rxclk_tx 0.0 [get_ports {ad9866_txsync}]

set_output_delay -add_delay -max -clock virt_ad9866_rxclk_tx 2.5 [get_ports {ad9866_tx[*]}]
set_output_delay -add_delay -min -clock virt_ad9866_rxclk_tx 0.0 [get_ports {ad9866_tx[*]}]


## AD9866 Other IO
set_false_path -to [get_ports {ad9866_sclk}]
set_false_path -to [get_ports {ad9866_sdio}]
set_false_path -from [get_ports {ad9866_sdo}]
set_false_path -to [get_ports {ad9866_sen_n}]
set_false_path -to [get_ports {ad9866_rst_n}]
set_false_path -to [get_ports {ad9866_mode}]
set_false_path -to [get_ports {ad9866_txquietn}]
set_false_path -to [get_ports {EER_PWM_out}]


## Additional timing constraints
				
set_max_delay -from ad9866_tx[4]~reg0	-to ad9866_tx[4]	10	

set_max_delay -from counter:counter_inst|lpm_counter:LPM_COUNTER_component|cntr_69j:auto_generated|counter_reg_bit[*]	-to counter:counter_inst|lpm_counter:LPM_COUNTER_component|cntr_69j:auto_generated|counter_reg_bit[*] 6
				
set_max_delay -from txFIFO:txFIFO_inst|dcfifo:dcfifo_component|dcfifo_tln1:auto_generated|dffpipe_re9:rs_brp|dffe12a[*]	-to spi_slave:spi_slave_rx2_inst|treg[*] 8
set_max_delay -from txFIFO:txFIFO_inst|dcfifo:dcfifo_component|dcfifo_tln1:auto_generated|dffpipe_re9:rs_bwp|dffe12a[*]	-to spi_slave:spi_slave_rx2_inst|treg[*] 8
set_max_delay -from txFIFO:txFIFO_EER_inst|dcfifo:dcfifo_component|dcfifo_tln1:auto_generated|dffpipe_re9:rs_bwp|dffe12a[*]	-to spi_slave:spi_slave_rx2_inst|treg[*] 8
set_max_delay -from txFIFO:txFIFO_EER_inst|dcfifo:dcfifo_component|dcfifo_tln1:auto_generated|dffpipe_re9:rs_brp|dffe12a[*]	-to spi_slave:spi_slave_rx2_inst|treg[*] 8
	
## end of constraints