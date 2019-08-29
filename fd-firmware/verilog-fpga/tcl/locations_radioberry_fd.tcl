set_location_assignment PIN_53 -to clk_76m8

set_location_assignment PIN_42 -to ptt_in
#set_location_assignment PIN_115 -to EER_PWM_out
set_location_assignment PIN_144 -to ptt_out

#set_location_assignment PIN_50 -to spi_ce[1]
#set_location_assignment PIN_51 -to spi_ce[0]
#set_location_assignment PIN_59 -to spi_miso
#set_location_assignment PIN_60 -to spi_mosi
#set_location_assignment PIN_58 -to spi_sck

#set_location_assignment PIN_31 -to rx_samples
#set_location_assignment PIN_54 -to pi_clk
#set_location_assignment PIN_113 -to pi_clk2
#set_location_assignment PIN_39 -to data[7]
#set_location_assignment PIN_43 -to data[6]
#set_location_assignment PIN_44 -to data[5]
#set_location_assignment PIN_33 -to data[4]
#set_location_assignment PIN_46 -to data[3]
#set_location_assignment PIN_13 -to data[2]
#set_location_assignment PIN_49 -to data[1]
#set_location_assignment PIN_32 -to data[0]

set_location_assignment PIN_111 -to ad9866_mode
set_location_assignment PIN_114 -to ad9866_rst_n
set_location_assignment PIN_65 -to ad9866_sen_n
set_location_assignment PIN_66 -to ad9866_sclk
set_location_assignment PIN_68 -to ad9866_sdio
set_location_assignment PIN_67 -to ad9866_sdo

set_location_assignment PIN_71 -to ad9866_rxclk
#set_location_assignment PIN_69 -to ad9866_clk
set_location_assignment PIN_80 -to ad9866_rx[0]
set_location_assignment PIN_83 -to ad9866_rx[1]
set_location_assignment PIN_85 -to ad9866_rx[2]
set_location_assignment PIN_86 -to ad9866_rx[3]
set_location_assignment PIN_87 -to ad9866_rx[4]
set_location_assignment PIN_98 -to ad9866_rx[5]

set_location_assignment PIN_77 -to ad9866_rxsync
set_location_assignment PIN_99 -to ad9866_tx[0]
set_location_assignment PIN_100 -to ad9866_tx[1]
set_location_assignment PIN_101 -to ad9866_tx[2]
set_location_assignment PIN_103 -to ad9866_tx[3]
set_location_assignment PIN_105 -to ad9866_tx[4]
set_location_assignment PIN_106 -to ad9866_tx[5]
set_location_assignment PIN_72 -to ad9866_txquietn
set_location_assignment PIN_76 -to ad9866_txsync

set_instance_assignment -name FAST_OUTPUT_REGISTER ON 	-to ad9866_tx[*]
set_instance_assignment -name FAST_OUTPUT_REGISTER ON 	-to ad9866_txsync
set_instance_assignment -name FAST_INPUT_REGISTER ON 	-to ad9866_rx[*]
set_instance_assignment -name FAST_INPUT_REGISTER ON 	-to ad9866_rxsync
set_instance_assignment -name FAST_OUTPUT_REGISTER ON 	-to ad9866_sdio
set_instance_assignment -name FAST_OUTPUT_REGISTER ON 	-to ad9866_sen_n
set_instance_assignment -name FAST_OUTPUT_REGISTER ON 	-to ad9866_sclk
set_instance_assignment -name FAST_OUTPUT_REGISTER ON	-to ad9866_rst_n
set_instance_assignment -name FAST_OUTPUT_REGISTER ON 	-to ad9866_txquietn

set_instance_assignment -name FAST_OUTPUT_ENABLE_REGISTER ON -to *

