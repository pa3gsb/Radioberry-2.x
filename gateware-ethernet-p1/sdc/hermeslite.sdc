
set_time_format -unit ns -decimal_places 3

## Clocks in Eth TX Domain 

create_clock -name phy_clk125 -period 125.000MHz	[get_ports phy_clk125]

create_generated_clock -source {ethpll_inst|altpll_component|auto_generated|pll1|inclk[0]} -duty_cycle 50.00 -name clock_125MHz {ethpll_inst|altpll_component|auto_generated|pll1|clk[0]}

create_generated_clock -source {ethpll_inst|altpll_component|auto_generated|pll1|inclk[0]} -phase 135.00 -duty_cycle 50.00 -name clock_90_125MHz {ethpll_inst|altpll_component|auto_generated|pll1|clk[1]}

create_generated_clock -source {ethpll_inst|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 50 -duty_cycle 50.00 -name clock_2_5MHz {ethpll_inst|altpll_component|auto_generated|pll1|clk[2]}

create_generated_clock -source {ethpll_inst|altpll_component|auto_generated|pll1|inclk[0]} -phase 99.0 -divide_by 5 -duty_cycle 50.00 -name clock_25MHz {ethpll_inst|altpll_component|auto_generated|pll1|clk[3]}

create_generated_clock -source {ethpll_inst|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 10 -duty_cycle 50.00 -name clock_12p5MHz {ethpll_inst|altpll_component|auto_generated|pll1|clk[4]}

create_generated_clock -source {ethpll_inst|altpll_component|auto_generated|pll1|clk[1]} -name clock_ethtxextfast {ethtxext_clkmux_i|auto_generated|clkctrl1|outclk}

create_generated_clock -source {ethpll_inst|altpll_component|auto_generated|pll1|clk[3]} -name clock_ethtxextslow {ethtxext_clkmux_i|auto_generated|clkctrl1|outclk} -add

set_clock_groups -exclusive -group {clock_ethtxextslow} -group {clock_ethtxextfast} 

create_generated_clock -source {ethpll_inst|altpll_component|auto_generated|pll1|clk[0]} -name clock_ethtxintfast {ethtxint_clkmux_i|auto_generated|clkctrl1|outclk}

create_generated_clock -source {ethpll_inst|altpll_component|auto_generated|pll1|clk[4]} -name clock_ethtxintslow {ethtxint_clkmux_i|auto_generated|clkctrl1|outclk} -add

set_clock_groups -exclusive -group {clock_ethtxintslow} -group {clock_ethtxintfast} 

create_generated_clock -name clock_txoutputfast -master_clock [get_clocks {clock_ethtxextfast}] -source [get_pins {ethtxext_clkmux_i|auto_generated|clkctrl1|outclk}] [get_ports {phy_tx_clk}]

create_generated_clock -name clock_txoutputslow -master_clock [get_clocks {clock_ethtxextslow}] -source [get_pins {ethtxext_clkmux_i|auto_generated|clkctrl1|outclk}] [get_ports {phy_tx_clk}] -add

set_clock_groups -exclusive -group {clock_txoutputslow} -group {clock_txoutputfast} 



## Clocks in Eth RX Domain 

create_clock -name virt_phy_rx_clk_fast	-period 8.000
create_clock -name virt_phy_rx_clk_slow	-period 40.000

set_clock_groups -exclusive -group {virt_phy_rx_clk_fast} -group {virt_phy_rx_clk_slow} 

create_clock -name phy_rx_clk -period 8.000	-waveform {2.0 6.0} [get_ports {phy_rx_clk}]

create_generated_clock -name clock_ethrxintfast -source [get_ports {phy_rx_clk}] {clock_ethrxint}
create_generated_clock -name clock_ethrxintslow -source [get_ports {phy_rx_clk}] {clock_ethrxint} -divide_by 10 -add
set_clock_groups -exclusive -group {clock_ethrxintslow} -group {clock_ethrxintfast} 


## Clocks in AD9866 Domain

create_clock -name rffe_ad9866_clk76p8 	-period 76.8MHz [get_ports rffe_ad9866_clk76p8]		
create_clock -name virt_ad9866_rxclk_tx -period 153.6MHz 
create_clock -name virt_ad9866_rxclk_rx -period 153.6MHz 

create_generated_clock -source {ad9866pll_inst|altpll_component|auto_generated|pll1|inclk[0]} -duty_cycle 50.00 -name clock_76p8MHz {ad9866pll_inst|altpll_component|auto_generated|pll1|clk[0]}

create_generated_clock -source {ad9866pll_inst|altpll_component|auto_generated|pll1|inclk[0]} -multiply_by 2 -duty_cycle 50.00 -name clock_153p6_mhz {ad9866pll_inst|altpll_component|auto_generated|pll1|clk[1]}

create_generated_clock -source {ad9866pll_inst|altpll_component|auto_generated|pll1|inclk[0]} -multiply_by 16 -divide_by 5 -duty_cycle 50.00 -name clock_245p76_mhz {ad9866pll_inst|altpll_component|auto_generated|pll1|clk[2]}

derive_clock_uncertainty


set_clock_groups -asynchronous -group { \
		phy_clk125 \
		clock_ethtxintfast clock_ethtxintslow \
		clock_ethtxextfast clock_ethtxextslow \
		clock_125MHz clock_90_125MHz clock_2_5MHz \
		clock_txoutputslow clock_txoutputfast \
	} -group { \
		phy_rx_clk \
		clock_ethrxintslow \
		clock_ethrxintfast \
	} -group { \
		clock_153p6_mhz rffe_ad9866_clk76p8 clock_76p8MHz \
	}



## Ethernet PHY TX per AN477, with PHY delay for TX disabled

set_output_delay  -max  1.0 -clock clock_txoutputfast [get_ports {phy_tx[*]}]
set_output_delay  -min -0.8 -clock clock_txoutputfast [get_ports {phy_tx[*]}]  -add_delay
set_output_delay  -max  1.0 -clock clock_txoutputfast [get_ports {phy_tx[*]}]  -clock_fall -add_delay
set_output_delay  -min -0.8 -clock clock_txoutputfast [get_ports {phy_tx[*]}]  -clock_fall -add_delay

set_output_delay  -max  1.0 -clock clock_txoutputfast [get_ports {phy_tx_en}] 
set_output_delay  -min -0.8 -clock clock_txoutputfast [get_ports {phy_tx_en}]  -add_delay
set_output_delay  -max  1.0 -clock clock_txoutputfast [get_ports {phy_tx_en}]  -clock_fall -add_delay
set_output_delay  -min -0.8 -clock clock_txoutputfast [get_ports {phy_tx_en}]  -clock_fall -add_delay

set_output_delay  -max  1.0 -clock clock_txoutputslow [get_ports {phy_tx[*]}]  -add_delay
set_output_delay  -min -0.8 -clock clock_txoutputslow [get_ports {phy_tx[*]}]  -add_delay
set_output_delay  -max  1.0 -clock clock_txoutputslow [get_ports {phy_tx[*]}]  -clock_fall -add_delay
set_output_delay  -min -0.8 -clock clock_txoutputslow [get_ports {phy_tx[*]}]  -clock_fall -add_delay

set_output_delay  -max  1.0 -clock clock_txoutputslow [get_ports {phy_tx_en}]  -add_delay
set_output_delay  -min -0.8 -clock clock_txoutputslow [get_ports {phy_tx_en}]  -add_delay
set_output_delay  -max  1.0 -clock clock_txoutputslow [get_ports {phy_tx_en}]  -clock_fall -add_delay
set_output_delay  -min -0.8 -clock clock_txoutputslow [get_ports {phy_tx_en}]  -clock_fall -add_delay

set_false_path -fall_from [get_clocks {clock_ethtxintfast}] -rise_to [get_clocks {clock_txoutputfast}] -setup
set_false_path -rise_from [get_clocks {clock_ethtxintfast}] -fall_to [get_clocks {clock_txoutputfast}] -setup
set_false_path -fall_from [get_clocks {clock_ethtxintfast}] -fall_to [get_clocks {clock_txoutputfast}] -hold
set_false_path -rise_from [get_clocks {clock_ethtxintfast}] -rise_to [get_clocks {clock_txoutputfast}] -hold

set_false_path -fall_from [get_clocks {clock_ethtxintslow}] -rise_to [get_clocks {clock_txoutputslow}] -setup
set_false_path -rise_from [get_clocks {clock_ethtxintslow}] -fall_to [get_clocks {clock_txoutputslow}] -setup
set_false_path -fall_from [get_clocks {clock_ethtxintslow}] -fall_to [get_clocks {clock_txoutputslow}] -hold
set_false_path -rise_from [get_clocks {clock_ethtxintslow}] -rise_to [get_clocks {clock_txoutputslow}] -hold

set_false_path -from [get_clocks {clock_ethtxintfast}] -to [get_clocks {clock_txoutputslow}]
set_false_path -from [get_clocks {clock_ethtxintslow}] -to [get_clocks {clock_txoutputfast}]



## Ethernet PHY RX per AN477, with PHY delay for RX enabled
## Clock delay added by KSZ9031 is 1.0 to 2.6 per datasheet table 7-1
## Clock is 90deg shifted, 2ns
## Max delay is 2.6-2.0 = 0.6
## Min delay is 1.0-2.0 = -1.0

set_input_delay  -max  0.6 -clock virt_phy_rx_clk_fast [get_ports {phy_rx[*]}]
set_input_delay  -min -1.0 -clock virt_phy_rx_clk_fast -add_delay [get_ports {phy_rx[*]}]
set_input_delay  -max  0.6 -clock virt_phy_rx_clk_fast -clock_fall -add_delay [get_ports {phy_rx[*]}]
set_input_delay  -min -1.0 -clock virt_phy_rx_clk_fast -clock_fall -add_delay [get_ports {phy_rx[*]}]

set_input_delay  -max  0.6 -clock virt_phy_rx_clk_fast [get_ports {phy_rx_dv}]
set_input_delay  -min -1.0 -clock virt_phy_rx_clk_fast -add_delay [get_ports {phy_rx_dv}]
set_input_delay  -max  0.6 -clock virt_phy_rx_clk_fast -clock_fall -add_delay [get_ports {phy_rx_dv}]
set_input_delay  -min -1.0 -clock virt_phy_rx_clk_fast -clock_fall -add_delay [get_ports {phy_rx_dv}]

set_false_path -fall_from  virt_phy_rx_clk_fast -rise_to clock_ethrxintfast -setup
set_false_path -rise_from  virt_phy_rx_clk_fast -fall_to clock_ethrxintfast -setup
set_false_path -fall_from  virt_phy_rx_clk_fast -fall_to clock_ethrxintfast -hold
set_false_path -rise_from  virt_phy_rx_clk_fast -rise_to clock_ethrxintfast -hold

set_false_path -from [get_clocks {virt_phy_rx_clk_fast}] -to [get_clocks {clock_ethrxintslow}]

set_input_delay  -max  0.6 -clock virt_phy_rx_clk_slow [get_ports {phy_rx[*]}] -add_delay
set_input_delay  -min -1.0 -clock virt_phy_rx_clk_slow -add_delay [get_ports {phy_rx[*]}]
set_input_delay  -max  0.6 -clock virt_phy_rx_clk_slow -clock_fall -add_delay [get_ports {phy_rx[*]}]
set_input_delay  -min -1.0 -clock virt_phy_rx_clk_slow -clock_fall -add_delay [get_ports {phy_rx[*]}]

set_input_delay  -max  0.6 -clock virt_phy_rx_clk_slow [get_ports {phy_rx_dv}] -add_delay
set_input_delay  -min -1.0 -clock virt_phy_rx_clk_slow -add_delay [get_ports {phy_rx_dv}]
set_input_delay  -max  0.6 -clock virt_phy_rx_clk_slow -clock_fall -add_delay [get_ports {phy_rx_dv}]
set_input_delay  -min -1.0 -clock virt_phy_rx_clk_slow -clock_fall -add_delay [get_ports {phy_rx_dv}]

set_false_path -fall_from  virt_phy_rx_clk_slow -rise_to clock_ethrxintslow -setup
set_false_path -rise_from  virt_phy_rx_clk_slow -fall_to clock_ethrxintslow -setup
set_false_path -fall_from  virt_phy_rx_clk_slow -fall_to clock_ethrxintslow -hold
set_false_path -rise_from  virt_phy_rx_clk_slow -rise_to clock_ethrxintslow -hold

set_false_path -from [get_clocks {virt_phy_rx_clk_slow}] -to [get_clocks {clock_ethrxintfast}]

## Misc PHY

#PHY PHY_MDIO Data in +/- 10nS setup and hold
set_input_delay  10  -clock clock_2_5MHz -reference_pin [get_ports phy_mdc] {phy_mdio}

#PHY (2.5MHz)
set_output_delay  10 -clock clock_2_5MHz -reference_pin [get_ports phy_mdc] {phy_mdio}

set_max_delay -from clock_2_5MHz -to clock_ethtxintfast 22

set_false_path -to [get_ports {phy_mdc}]

#set_false_path -from {ethtxint_clkmux_i|auto_generated|ena_reg}
#set_false_path -from {ethtxext_clkmux_i|auto_generated|ena_reg}

#set_input_delay -clock clock_2_5MHz -max 3 [get_ports {phy_rst_n}] 
#set_input_delay -clock clock_2_5MHz -min 2 [get_ports {phy_rst_n}] 
set_false_path -from [get_ports {phy_rst_n}]

set_false_path -from [get_keepers {network:network_inst|phy_cfg:phy_cfg_inst|speed[1]}] 



## IO

#set_output_delay -clock clock_76p8MHz -max 3 [get_ports {pwr_*}] 
#set_output_delay -clock clock_76p8MHz -min 2 [get_ports {pwr_*}] 
set_false_path -to [get_ports {pwr_*}]

#set_output_delay -clock clock_76p8MHz -max 3 [get_ports {clk_*}] 
#set_output_delay -clock clock_76p8MHz -min 2 [get_ports {clk_*}] 
#set_input_delay -clock clock_76p8MHz -max 3 [get_ports {clk_s*}] 
#set_input_delay -clock clock_76p8MHz -min 2 [get_ports {clk_s*}] 
set_false_path -to [get_ports {clk_*}] 
set_false_path -from [get_ports {clk_s*}] 

#set_output_delay -clock clock_76p8MHz -max 3 [get_ports {io_led_*}] 
#set_output_delay -clock clock_76p8MHz -min 2 [get_ports {io_led_*}] 
set_false_path -to [get_ports {io_led_*}]

#set_input_delay -clock clock_76p8MHz -max 3 [get_ports {io_lvds_*}] 
#set_input_delay -clock clock_76p8MHz -min 2 [get_ports {io_lvds_*}] 
set_false_path -from [get_ports {io_lvds_*}]

#set_input_delay -clock clock_76p8MHz -max 3 [get_ports {io_cn*}] 
#set_input_delay -clock clock_76p8MHz -min 2 [get_ports {io_cn*}] 
set_false_path -from [get_ports {io_cn*}]

#set_input_delay -clock clock_76p8MHz -max 3 [get_ports {io_adc_s*}] 
#set_input_delay -clock clock_76p8MHz -min 2 [get_ports {io_adc_s*}] 
#set_output_delay -clock clock_76p8MHz -max 3 [get_ports {io_adc_s*}] 
#set_output_delay -clock clock_76p8MHz -min 2 [get_ports {io_adc_s*}] 
set_false_path -to [get_ports {io_adc_s*}]
set_false_path -from [get_ports {io_adc_s*}]

#set_input_delay -clock clock_76p8MHz -max 3 [get_ports {io_s*}] 
#set_input_delay -clock clock_76p8MHz -min 2 [get_ports {io_s*}] 
#set_output_delay -clock clock_76p8MHz -max 3 [get_ports {io_s*}] 
#set_output_delay -clock clock_76p8MHz -min 2 [get_ports {io_s*}] 
set_false_path -to [get_ports {io_s*}]
set_false_path -from [get_ports {io_s*}]

#set_input_delay -clock clock_76p8MHz -max 3 [get_ports {io_db1*}] 
#set_input_delay -clock clock_76p8MHz -min 2 [get_ports {io_db1*}] 
#set_output_delay -clock clock_76p8MHz -max 3 [get_ports {io_db1_5}] 
#set_output_delay -clock clock_76p8MHz -min 2 [get_ports {io_db1_5}] 
set_false_path -to [get_ports {io_db1_5}]
set_false_path -from [get_ports {io_db1_2}]
set_false_path -from [get_ports {io_db1_3}]
set_false_path -from [get_ports {io_db1_4}]
set_false_path -from [get_ports {io_db1_5}]

#set_input_delay -clock clock_76p8MHz -max 3 [get_ports {io_phone_*}] 
#set_input_delay -clock clock_76p8MHz -min 2 [get_ports {io_phone_*}] 
set_false_path -from [get_ports {io_phone_*}]

#set_input_delay -clock clock_76p8MHz -max 3 [get_ports {io_tp*}] 
#set_input_delay -clock clock_76p8MHz -min 2 [get_ports {io_tp*}] 
set_false_path -from [get_ports {io_tp*}]

#set_output_delay -clock clock_76p8MHz -max 5 [get_ports {pa_*}] 
#set_output_delay -clock clock_76p8MHz -min 2 [get_ports {pa_*}] 
set_false_path -to [get_ports {pa_*}]


## Multicycle in MAC

set_multicycle_path -from [get_keepers {network:network_inst|dhcp:dhcp_inst|length[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -setup -start 3
set_multicycle_path -from [get_keepers {network:network_inst|dhcp:dhcp_inst|length[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -hold -start 2

set_multicycle_path -from [get_keepers {network:network_inst|tx_protocol*}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -setup -start 3
set_multicycle_path -from [get_keepers {network:network_inst|tx_protocol*}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -hold -start 2

set_multicycle_path -from [get_keepers {network:network_inst|cdc_sync:cdc_sync_inst7|sigb[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -setup -start 2
set_multicycle_path -from [get_keepers {network:network_inst|cdc_sync:cdc_sync_inst7|sigb[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -hold -start 1

set_multicycle_path -from [get_keepers {network:network_inst|run_destination_ip[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -setup -start 2
set_multicycle_path -from [get_keepers {network:network_inst|run_destination_ip[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -hold -start 1

set_multicycle_path -from [get_keepers {network:network_inst|arp:arp_inst|tx_byte_no[*]}] -to [get_keepers {network:network_inst|mac_send:mac_send_inst|shift_reg[*]}] -setup -start 2
set_multicycle_path -from [get_keepers {network:network_inst|arp:arp_inst|tx_byte_no[*]}] -to [get_keepers {network:network_inst|mac_send:mac_send_inst|shift_reg[*]}] -hold -start 1

#set_multicycle_path -from [get_keepers {Tx_send:tx_send_inst|udp_tx_length[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -setup -start 3
#set_multicycle_path -from [get_keepers {Tx_send:tx_send_inst|udp_tx_length[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -hold -start 2

set_multicycle_path -from [get_keepers {usopenhpsdr1:usopenhpsdr1_i|udp_tx_length[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -setup -start 3
set_multicycle_path -from [get_keepers {usopenhpsdr1:usopenhpsdr1_i|udp_tx_length[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -hold -start 2

#set_multicycle_path -from [get_keepers {network:network_inst|icmp:icmp_inst|length[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -setup -start 3
#set_multicycle_path -from [get_keepers {network:network_inst|icmp:icmp_inst|length[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -hold -start 2

#set_multicycle_path -from [get_keepers {network:network_inst|icmp:icmp_inst|destination_ip[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -setup -start 3
#set_multicycle_path -from [get_keepers {network:network_inst|icmp:icmp_inst|destination_ip[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -hold -start 2

#set_multicycle_path -from [get_keepers {network:network_inst|dhcp:dhcp_inst|destination_ip[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -setup -start 3
#set_multicycle_path -from [get_keepers {network:network_inst|dhcp:dhcp_inst|destination_ip[*]}] -to [get_keepers {network:network_inst|ip_send:ip_send_inst|shift_reg[*]}] -hold -start 2

#set_multicycle_path -from {network:network_inst|icmp:icmp_inst|icmp_fifo:icmp_fifo_inst|*} -to {network:network_inst|ip_send:ip_send_inst|shift_reg[*]} -setup -start 2
#set_multicycle_path -from {network:network_inst|icmp:icmp_inst|icmp_fifo:icmp_fifo_inst|*} -to {network:network_inst|ip_send:ip_send_inst|shift_reg[*]} -hold -start 1

#set_multicycle_path -from {network:network_inst|udp_send:udp_send_inst|byte_no[*]} -to {network:network_inst|mac_send:mac_send_inst|shift_reg[*]} -setup -start 2
#set_multicycle_path -from {network:network_inst|udp_send:udp_send_inst|byte_no[*]} -to {network:network_inst|mac_send:mac_send_inst|shift_reg[*]} -hold -start 1

#set_multicycle_path -from {network:network_inst|tx_protocol*} -to {network:network_inst|mac_send:mac_send_inst|shift_reg[*]} -setup -start 2
#set_multicycle_path -from {network:network_inst|tx_protocol*} -to {network:network_inst|mac_send:mac_send_inst|shift_reg[*]} -hold -start 1

#set_multicycle_path -from {network:network_inst|icmp:icmp_inst|sending*} -to {network:network_inst|mac_send:mac_send_inst|shift_reg[*]} -setup -start 2
#set_multicycle_path -from {network:network_inst|icmp:icmp_inst|sending*} -to {network:network_inst|mac_send:mac_send_inst|shift_reg[*]} -hold -start 1



## Multicycle for frequency computation

##set_multicycle_path 2 -from {radio_i|wbs_dat_i[*]} -to {radio_i|freqcompp[*][*]} -setup -end 
##set_multicycle_path 1 -from {radio_i|wbs_dat_i[*]} -to {radio_i|freqcompp[*][*]} -hold -end 




## AD9866 RX Path
## See http://billauer.co.il/blog/2017/04/altera-intel-fpga-io-ff-packing/

set_input_delay -add_delay -max -clock virt_ad9866_rxclk_rx 4.5 [get_ports {rffe_ad9866_rxsync}]
set_input_delay -add_delay -min -clock virt_ad9866_rxclk_rx 0.0 [get_ports {rffe_ad9866_rxsync}]

set_input_delay -add_delay -max -clock virt_ad9866_rxclk_rx 4.5 [get_ports {rffe_ad9866_rx[*]}]
set_input_delay -add_delay -min -clock virt_ad9866_rxclk_rx 0.0 [get_ports {rffe_ad9866_rx[*]}]


## AD9866 TX Path

set_output_delay -add_delay -max -clock virt_ad9866_rxclk_tx 1.4 [get_ports {rffe_ad9866_txsync}]
set_output_delay -add_delay -min -clock virt_ad9866_rxclk_tx -1.3 [get_ports {rffe_ad9866_txsync}]

set_output_delay -add_delay -max -clock virt_ad9866_rxclk_tx 1.4 [get_ports {rffe_ad9866_tx[*]}]
set_output_delay -add_delay -min -clock virt_ad9866_rxclk_tx -1.3 [get_ports {rffe_ad9866_tx[*]}]



##set_max_delay -from {IF_Rx_ctrl_*} -to {freqcompp*} 15.9

## AD9866 Other IO

set_false_path -to [get_ports {rffe_ad9866_rst_n}]
set_false_path -to [get_ports {rffe_ad9866_txquiet_n}]
set_false_path -to [get_ports {rffe_ad9866_sdio}]
set_false_path -to [get_ports {rffe_ad9866_sclk}]
set_false_path -to [get_ports {rffe_ad9866_sen_n}]
set_false_path -to [get_ports {rffe_rfsw_sel}]
set_false_path -to [get_ports {rffe_ad9866_mode}]
set_false_path -to [get_ports {rffe_ad9866_pga*}]

## Multicycle for FIR

set_multicycle_path -from [get_clocks {clock_153p6_mhz}] -to [get_clocks {clock_76p8MHz}] -setup -start 2
set_multicycle_path -from [get_clocks {clock_153p6_mhz}] -to [get_clocks {clock_76p8MHz}] -hold -start 2

