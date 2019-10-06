


#hermeslite....


set_location_assignment PIN_43 -to debug1
set_location_assignment PIN_46 -to debug2
set_location_assignment PIN_44 -to debug3
set_location_assignment PIN_49 -to debug4

set_location_assignment PIN_142 -to phy_mdio
set_location_assignment PIN_141 -to phy_mdc

set_location_assignment PIN_125 -to phy_tx[0]
set_location_assignment PIN_121 -to phy_tx[1]
set_location_assignment PIN_132 -to phy_tx_en
set_location_assignment PIN_135 -to phy_rx[0]
set_location_assignment PIN_133 -to phy_rx[1]
set_location_assignment PIN_136 -to phy_rx_dv
set_location_assignment PIN_137 -to phy_clk

set_location_assignment PIN_99 -to rffe_ad9866_tx[0]
set_location_assignment PIN_100 -to rffe_ad9866_tx[1]
set_location_assignment PIN_101 -to rffe_ad9866_tx[2]
set_location_assignment PIN_103 -to rffe_ad9866_tx[3]
set_location_assignment PIN_105 -to rffe_ad9866_tx[4]
set_location_assignment PIN_106 -to rffe_ad9866_tx[5]

set_location_assignment PIN_111 -to rffe_ad9866_mode

set_location_assignment PIN_80 -to rffe_ad9866_rx[0]
set_location_assignment PIN_83 -to rffe_ad9866_rx[1]
set_location_assignment PIN_85 -to rffe_ad9866_rx[2]
set_location_assignment PIN_86 -to rffe_ad9866_rx[3]
set_location_assignment PIN_87 -to rffe_ad9866_rx[4]
set_location_assignment PIN_98 -to rffe_ad9866_rx[5]

set_location_assignment PIN_53 -to rffe_ad9866_clk76p8

set_location_assignment PIN_114 -to rffe_ad9866_rst_n
set_location_assignment PIN_65 -to rffe_ad9866_sen_n
set_location_assignment PIN_66 -to rffe_ad9866_sclk
set_location_assignment PIN_68 -to rffe_ad9866_sdio
set_location_assignment PIN_67 -to rffe_ad9866_sdo
set_location_assignment PIN_69 -to rffe_ad9866_rxclk
set_location_assignment PIN_77 -to rffe_ad9866_rxsync
set_location_assignment PIN_72 -to rffe_ad9866_txquiet_n
set_location_assignment PIN_76 -to rffe_ad9866_txsync



set_instance_assignment -name FAST_OUTPUT_REGISTER ON -to rffe_ad9866_tx[*]
set_instance_assignment -name FAST_OUTPUT_REGISTER ON -to rffe_ad9866_txsync
set_instance_assignment -name FAST_INPUT_REGISTER ON -to rffe_ad9866_rx[*]
set_instance_assignment -name FAST_INPUT_REGISTER ON -to rffe_ad9866_rxsync
set_instance_assignment -name FAST_OUTPUT_REGISTER ON -to rffe_ad9866_sdio
set_instance_assignment -name FAST_OUTPUT_REGISTER ON -to rffe_ad9866_sen_n
set_instance_assignment -name FAST_OUTPUT_REGISTER ON -to rffe_ad9866_sclk
set_instance_assignment -name FAST_OUTPUT_REGISTER ON -to rffe_ad9866_rst_n
set_instance_assignment -name FAST_OUTPUT_REGISTER ON -to rffe_ad9866_txquiet_n
set_instance_assignment -name FAST_OUTPUT_REGISTER ON -to rffe_rfsw_sel



set_instance_assignment -name FAST_OUTPUT_ENABLE_REGISTER ON -to *