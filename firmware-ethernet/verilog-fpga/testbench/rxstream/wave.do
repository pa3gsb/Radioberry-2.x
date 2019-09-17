onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -expand -group tb -radix unsigned ./rxstream_inst/clk
add wave -noupdate -expand -group tb -radix unsigned ./rxstream_inst/run
add wave -noupdate -expand -group tb -radix unsigned ./rxstream_inst/have_ip
add wave -noupdate -expand -group tb -radix unsigned ./rxstream_inst/udp_tx_request
add wave -noupdate -expand -group tb -radix unsigned ./rxstream_inst/udp_tx_enable
add wave -noupdate -expand -group tb -radix hexadecimal ./rxstream_inst/udp_tx_data
add wave -noupdate -expand -group tb -radix unsigned ./rxstream_inst/rx_request
add wave -noupdate -expand -group tb -radix hexadecimal ./rxstream_inst/rx_data

TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {816212 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 177
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {0 ps} {7686 ns}
