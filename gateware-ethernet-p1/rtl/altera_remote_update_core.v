//altremote_update CBX_AUTO_BLACKBOX="ALL" CBX_SINGLE_OUTPUT_FILE="ON" check_app_pof="false" config_device_addr_width=24 DEVICE_FAMILY="Cyclone IV E" in_data_width=24 is_epcq="false" operation_mode="remote" out_data_width=29 busy clock ctl_nupdt data_in data_out param read_param read_source reconfig reset reset_timer write_param
//VERSION_BEGIN 18.1 cbx_altremote_update 2018:09:12:13:04:09:SJ cbx_cycloneii 2018:09:12:13:04:09:SJ cbx_lpm_add_sub 2018:09:12:13:04:09:SJ cbx_lpm_compare 2018:09:12:13:04:09:SJ cbx_lpm_counter 2018:09:12:13:04:09:SJ cbx_lpm_decode 2018:09:12:13:04:09:SJ cbx_lpm_shiftreg 2018:09:12:13:04:09:SJ cbx_mgl 2018:09:12:14:15:07:SJ cbx_nadder 2018:09:12:13:04:09:SJ cbx_nightfury 2018:09:12:13:04:09:SJ cbx_stratix 2018:09:12:13:04:09:SJ cbx_stratixii 2018:09:12:13:04:09:SJ  VERSION_END
// synthesis VERILOG_INPUT_VERSION VERILOG_2001
// altera message_off 10463



// Copyright (C) 2018  Intel Corporation. All rights reserved.
//  Your use of Intel Corporation's design tools, logic functions 
//  and other software and tools, and its AMPP partner logic 
//  functions, and any output files from any of the foregoing 
//  (including device programming or simulation files), and any 
//  associated documentation or information are expressly subject 
//  to the terms and conditions of the Intel Program License 
//  Subscription Agreement, the Intel Quartus Prime License Agreement,
//  the Intel FPGA IP License Agreement, or other applicable license
//  agreement, including, without limitation, that your use is for
//  the sole purpose of programming logic devices manufactured by
//  Intel and sold by Intel or its authorized distributors.  Please
//  refer to the applicable agreement for further details.



//synthesis_resources = cycloneive_rublock 1 lpm_counter 2 reg 62 
//synopsys translate_off
`timescale 1 ps / 1 ps
//synopsys translate_on
(* ALTERA_ATTRIBUTE = {"suppress_da_rule_internal=c104;suppress_da_rule_internal=C101;suppress_da_rule_internal=C103"} *)
module  altera_remote_update_core
	( 
	busy,
	clock,
	ctl_nupdt,
	data_in,
	data_out,
	param,
	read_param,
	read_source,
	reconfig,
	reset,
	reset_timer,
	write_param) /* synthesis synthesis_clearbox=1 */;
	output   busy;
	input   clock;
	input   ctl_nupdt;
	input   [23:0]  data_in;
	output   [28:0]  data_out;
	input   [2:0]  param;
	input   read_param;
	input   [1:0]  read_source;
	input   reconfig;
	input   reset;
	input   reset_timer;
	input   write_param;
`ifndef ALTERA_RESERVED_QIS
// synopsys translate_off
`endif
	tri0   ctl_nupdt;
	tri0   [23:0]  data_in;
	tri0   [2:0]  param;
	tri0   read_param;
	tri0   [1:0]  read_source;
	tri0   reconfig;
	tri0   reset_timer;
	tri0   write_param;
`ifndef ALTERA_RESERVED_QIS
// synopsys translate_on
`endif

	reg	[0:0]	check_busy_dffe;
	reg	[0:0]	dffe1a0;
	reg	[0:0]	dffe1a1;
	wire	[1:0]	wire_dffe1a_ena;
	reg	[0:0]	dffe2a0;
	reg	[0:0]	dffe2a1;
	reg	[0:0]	dffe2a2;
	wire	[2:0]	wire_dffe2a_ena;
	reg	[0:0]	dffe3a0;
	reg	[0:0]	dffe3a1;
	reg	[0:0]	dffe3a2;
	wire	[2:0]	wire_dffe3a_ena;
	reg	[28:0]	dffe7a;
	wire	[28:0]	wire_dffe7a_ena;
	reg	dffe8;
	reg	[6:0]	dffe9a;
	wire	[6:0]	wire_dffe9a_ena;
	reg	idle_state;
	reg	idle_write_wait;
	reg	read_address_state;
	wire	wire_read_address_state_ena;
	reg	read_data_state;
	reg	read_init_counter_state;
	reg	read_init_state;
	reg	read_post_state;
	reg	read_pre_data_state;
	reg	read_source_update_state;
	reg	write_data_state;
	reg	write_init_counter_state;
	reg	write_init_state;
	reg	write_load_state;
	reg	write_post_data_state;
	reg	write_pre_data_state;
	reg	write_source_update_state;
	reg	write_wait_state;
	wire  [5:0]   wire_cntr5_q;
	wire  [4:0]   wire_cntr6_q;
	wire  wire_sd4_regout;
	wire  bit_counter_all_done;
	wire  bit_counter_clear;
	wire  bit_counter_enable;
	wire  [5:0]  bit_counter_param_start;
	wire  bit_counter_param_start_match;
	wire  [6:0]  combine_port;
	wire  global_gnd;
	wire  global_vcc;
	wire  idle;
	wire  [6:0]  param_decoder_param_latch;
	wire  [22:0]  param_decoder_select;
	wire  power_up;
	wire  read_address;
	wire  read_data;
	wire  read_init;
	wire  read_init_counter;
	wire  read_post;
	wire  read_pre_data;
	wire  read_source_update;
	wire  rsource_load;
	wire  [1:0]  rsource_parallel_in;
	wire  rsource_serial_out;
	wire  rsource_shift_enable;
	wire  [2:0]  rsource_state_par_ini;
	wire  rsource_update_done;
	wire  rublock_captnupdt;
	wire  rublock_clock;
	wire  rublock_reconfig;
	wire  rublock_reconfig_st;
	wire  rublock_regin;
	wire  rublock_regout;
	wire  rublock_regout_reg;
	wire  rublock_shiftnld;
	wire  select_shift_nloop;
	wire  shift_reg_clear;
	wire  shift_reg_load_enable;
	wire  shift_reg_serial_in;
	wire  shift_reg_serial_out;
	wire  shift_reg_shift_enable;
	wire  [5:0]  start_bit_decoder_out;
	wire  [22:0]  start_bit_decoder_param_select;
	wire  [1:0]  w4w;
	wire  [5:0]  w53w;
	wire  [4:0]  w83w;
	wire  width_counter_all_done;
	wire  width_counter_clear;
	wire  width_counter_enable;
	wire  [4:0]  width_counter_param_width;
	wire  width_counter_param_width_match;
	wire  [4:0]  width_decoder_out;
	wire  [22:0]  width_decoder_param_select;
	wire  write_data;
	wire  write_init;
	wire  write_init_counter;
	wire  write_load;
	wire  write_post_data;
	wire  write_pre_data;
	wire  write_source_update;
	wire  write_wait;
	wire  [2:0]  wsource_state_par_ini;
	wire  wsource_update_done;

	// synopsys translate_off
	initial 
		 check_busy_dffe[0:0] = 0;
	// synopsys translate_on
	// synopsys translate_off
	initial
		dffe1a0 = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe1a0 <= 1'b0;
		else if  (wire_dffe1a_ena[0:0] == 1'b1)   dffe1a0 <= ((rsource_load & rsource_parallel_in[0]) | ((~ rsource_load) & dffe1a1[0:0]));
	// synopsys translate_off
	initial
		dffe1a1 = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe1a1 <= 1'b0;
		else if  (wire_dffe1a_ena[1:1] == 1'b1)   dffe1a1 <= (rsource_parallel_in[1] & rsource_load);
	assign
		wire_dffe1a_ena = {2{(rsource_load | rsource_shift_enable)}};
	// synopsys translate_off
	initial
		dffe2a0 = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe2a0 <= 1'b0;
		else if  (wire_dffe2a_ena[0:0] == 1'b1)   dffe2a0 <= ((rsource_load & rsource_state_par_ini[0]) | ((~ rsource_load) & dffe2a1[0:0]));
	// synopsys translate_off
	initial
		dffe2a1 = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe2a1 <= 1'b0;
		else if  (wire_dffe2a_ena[1:1] == 1'b1)   dffe2a1 <= ((rsource_load & rsource_state_par_ini[1]) | ((~ rsource_load) & dffe2a2[0:0]));
	// synopsys translate_off
	initial
		dffe2a2 = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe2a2 <= 1'b0;
		else if  (wire_dffe2a_ena[2:2] == 1'b1)   dffe2a2 <= (rsource_state_par_ini[2] & rsource_load);
	assign
		wire_dffe2a_ena = {3{(rsource_load | global_vcc)}};
	// synopsys translate_off
	initial
		dffe3a0 = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe3a0 <= 1'b0;
		else if  (wire_dffe3a_ena[0:0] == 1'b1)   dffe3a0 <= ((rsource_load & wsource_state_par_ini[0]) | ((~ rsource_load) & dffe3a1[0:0]));
	// synopsys translate_off
	initial
		dffe3a1 = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe3a1 <= 1'b0;
		else if  (wire_dffe3a_ena[1:1] == 1'b1)   dffe3a1 <= ((rsource_load & wsource_state_par_ini[1]) | ((~ rsource_load) & dffe3a2[0:0]));
	// synopsys translate_off
	initial
		dffe3a2 = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe3a2 <= 1'b0;
		else if  (wire_dffe3a_ena[2:2] == 1'b1)   dffe3a2 <= (wsource_state_par_ini[2] & rsource_load);
	assign
		wire_dffe3a_ena = {3{(rsource_load | global_vcc)}};
	// synopsys translate_off
	initial
		dffe7a[0:0] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[0:0] <= 1'b0;
		else if  (wire_dffe7a_ena[0:0] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[0:0] <= 1'b0;
			else  dffe7a[0:0] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[2]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[0]))) | ((~ shift_reg_load_enable) & dffe7a[1:1]));
	// synopsys translate_off
	initial
		dffe7a[1:1] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[1:1] <= 1'b0;
		else if  (wire_dffe7a_ena[1:1] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[1:1] <= 1'b0;
			else  dffe7a[1:1] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[3]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[1]))) | ((~ shift_reg_load_enable) & dffe7a[2:2]));
	// synopsys translate_off
	initial
		dffe7a[2:2] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[2:2] <= 1'b0;
		else if  (wire_dffe7a_ena[2:2] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[2:2] <= 1'b0;
			else  dffe7a[2:2] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[4]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[2]))) | ((~ shift_reg_load_enable) & dffe7a[3:3]));
	// synopsys translate_off
	initial
		dffe7a[3:3] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[3:3] <= 1'b0;
		else if  (wire_dffe7a_ena[3:3] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[3:3] <= 1'b0;
			else  dffe7a[3:3] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[5]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[3]))) | ((~ shift_reg_load_enable) & dffe7a[4:4]));
	// synopsys translate_off
	initial
		dffe7a[4:4] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[4:4] <= 1'b0;
		else if  (wire_dffe7a_ena[4:4] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[4:4] <= 1'b0;
			else  dffe7a[4:4] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[6]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[4]))) | ((~ shift_reg_load_enable) & dffe7a[5:5]));
	// synopsys translate_off
	initial
		dffe7a[5:5] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[5:5] <= 1'b0;
		else if  (wire_dffe7a_ena[5:5] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[5:5] <= 1'b0;
			else  dffe7a[5:5] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[7]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[5]))) | ((~ shift_reg_load_enable) & dffe7a[6:6]));
	// synopsys translate_off
	initial
		dffe7a[6:6] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[6:6] <= 1'b0;
		else if  (wire_dffe7a_ena[6:6] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[6:6] <= 1'b0;
			else  dffe7a[6:6] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[8]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[6]))) | ((~ shift_reg_load_enable) & dffe7a[7:7]));
	// synopsys translate_off
	initial
		dffe7a[7:7] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[7:7] <= 1'b0;
		else if  (wire_dffe7a_ena[7:7] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[7:7] <= 1'b0;
			else  dffe7a[7:7] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[9]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[7]))) | ((~ shift_reg_load_enable) & dffe7a[8:8]));
	// synopsys translate_off
	initial
		dffe7a[8:8] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[8:8] <= 1'b0;
		else if  (wire_dffe7a_ena[8:8] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[8:8] <= 1'b0;
			else  dffe7a[8:8] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[10]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[8]))) | ((~ shift_reg_load_enable) & dffe7a[9:9]));
	// synopsys translate_off
	initial
		dffe7a[9:9] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[9:9] <= 1'b0;
		else if  (wire_dffe7a_ena[9:9] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[9:9] <= 1'b0;
			else  dffe7a[9:9] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[11]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[9]))) | ((~ shift_reg_load_enable) & dffe7a[10:10]));
	// synopsys translate_off
	initial
		dffe7a[10:10] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[10:10] <= 1'b0;
		else if  (wire_dffe7a_ena[10:10] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[10:10] <= 1'b0;
			else  dffe7a[10:10] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[12]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[10]))) | ((~ shift_reg_load_enable) & dffe7a[11:11]));
	// synopsys translate_off
	initial
		dffe7a[11:11] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[11:11] <= 1'b0;
		else if  (wire_dffe7a_ena[11:11] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[11:11] <= 1'b0;
			else  dffe7a[11:11] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[13]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[11]))) | ((~ shift_reg_load_enable) & dffe7a[12:12]));
	// synopsys translate_off
	initial
		dffe7a[12:12] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[12:12] <= 1'b0;
		else if  (wire_dffe7a_ena[12:12] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[12:12] <= 1'b0;
			else  dffe7a[12:12] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[14]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[12]))) | ((~ shift_reg_load_enable) & dffe7a[13:13]));
	// synopsys translate_off
	initial
		dffe7a[13:13] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[13:13] <= 1'b0;
		else if  (wire_dffe7a_ena[13:13] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[13:13] <= 1'b0;
			else  dffe7a[13:13] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[15]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[13]))) | ((~ shift_reg_load_enable) & dffe7a[14:14]));
	// synopsys translate_off
	initial
		dffe7a[14:14] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[14:14] <= 1'b0;
		else if  (wire_dffe7a_ena[14:14] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[14:14] <= 1'b0;
			else  dffe7a[14:14] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[16]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[14]))) | ((~ shift_reg_load_enable) & dffe7a[15:15]));
	// synopsys translate_off
	initial
		dffe7a[15:15] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[15:15] <= 1'b0;
		else if  (wire_dffe7a_ena[15:15] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[15:15] <= 1'b0;
			else  dffe7a[15:15] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[17]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[15]))) | ((~ shift_reg_load_enable) & dffe7a[16:16]));
	// synopsys translate_off
	initial
		dffe7a[16:16] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[16:16] <= 1'b0;
		else if  (wire_dffe7a_ena[16:16] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[16:16] <= 1'b0;
			else  dffe7a[16:16] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[18]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[16]))) | ((~ shift_reg_load_enable) & dffe7a[17:17]));
	// synopsys translate_off
	initial
		dffe7a[17:17] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[17:17] <= 1'b0;
		else if  (wire_dffe7a_ena[17:17] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[17:17] <= 1'b0;
			else  dffe7a[17:17] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[19]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[17]))) | ((~ shift_reg_load_enable) & dffe7a[18:18]));
	// synopsys translate_off
	initial
		dffe7a[18:18] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[18:18] <= 1'b0;
		else if  (wire_dffe7a_ena[18:18] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[18:18] <= 1'b0;
			else  dffe7a[18:18] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[20]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[18]))) | ((~ shift_reg_load_enable) & dffe7a[19:19]));
	// synopsys translate_off
	initial
		dffe7a[19:19] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[19:19] <= 1'b0;
		else if  (wire_dffe7a_ena[19:19] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[19:19] <= 1'b0;
			else  dffe7a[19:19] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[21]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[19]))) | ((~ shift_reg_load_enable) & dffe7a[20:20]));
	// synopsys translate_off
	initial
		dffe7a[20:20] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[20:20] <= 1'b0;
		else if  (wire_dffe7a_ena[20:20] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[20:20] <= 1'b0;
			else  dffe7a[20:20] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[22]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[20]))) | ((~ shift_reg_load_enable) & dffe7a[21:21]));
	// synopsys translate_off
	initial
		dffe7a[21:21] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[21:21] <= 1'b0;
		else if  (wire_dffe7a_ena[21:21] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[21:21] <= 1'b0;
			else  dffe7a[21:21] <= ((shift_reg_load_enable & ((((param[2] & (~ param[1])) & (~ param[0])) & data_in[23]) | ((~ ((param[2] & (~ param[1])) & (~ param[0]))) & data_in[21]))) | ((~ shift_reg_load_enable) & dffe7a[22:22]));
	// synopsys translate_off
	initial
		dffe7a[22:22] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[22:22] <= 1'b0;
		else if  (wire_dffe7a_ena[22:22] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[22:22] <= 1'b0;
			else  dffe7a[22:22] <= ((~ shift_reg_load_enable) & dffe7a[23:23]);
	// synopsys translate_off
	initial
		dffe7a[23:23] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[23:23] <= 1'b0;
		else if  (wire_dffe7a_ena[23:23] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[23:23] <= 1'b0;
			else  dffe7a[23:23] <= ((~ shift_reg_load_enable) & dffe7a[24:24]);
	// synopsys translate_off
	initial
		dffe7a[24:24] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[24:24] <= 1'b0;
		else if  (wire_dffe7a_ena[24:24] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[24:24] <= 1'b0;
			else  dffe7a[24:24] <= ((~ shift_reg_load_enable) & dffe7a[25:25]);
	// synopsys translate_off
	initial
		dffe7a[25:25] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[25:25] <= 1'b0;
		else if  (wire_dffe7a_ena[25:25] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[25:25] <= 1'b0;
			else  dffe7a[25:25] <= ((~ shift_reg_load_enable) & dffe7a[26:26]);
	// synopsys translate_off
	initial
		dffe7a[26:26] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[26:26] <= 1'b0;
		else if  (wire_dffe7a_ena[26:26] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[26:26] <= 1'b0;
			else  dffe7a[26:26] <= ((~ shift_reg_load_enable) & dffe7a[27:27]);
	// synopsys translate_off
	initial
		dffe7a[27:27] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[27:27] <= 1'b0;
		else if  (wire_dffe7a_ena[27:27] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[27:27] <= 1'b0;
			else  dffe7a[27:27] <= ((~ shift_reg_load_enable) & dffe7a[28:28]);
	// synopsys translate_off
	initial
		dffe7a[28:28] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe7a[28:28] <= 1'b0;
		else if  (wire_dffe7a_ena[28:28] == 1'b1) 
			if (shift_reg_clear == 1'b1) dffe7a[28:28] <= 1'b0;
			else  dffe7a[28:28] <= ((~ shift_reg_load_enable) & shift_reg_serial_in);
	assign
		wire_dffe7a_ena = {29{((shift_reg_load_enable | shift_reg_shift_enable) | shift_reg_clear)}};
	// synopsys translate_off
	initial
		dffe8 = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe8 <= 1'b0;
		else  dffe8 <= rublock_regout;
	// synopsys translate_off
	initial
		dffe9a[0:0] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe9a[0:0] <= 1'b0;
		else if  (wire_dffe9a_ena[0:0] == 1'b1)   dffe9a[0:0] <= combine_port[0:0];
	// synopsys translate_off
	initial
		dffe9a[1:1] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe9a[1:1] <= 1'b0;
		else if  (wire_dffe9a_ena[1:1] == 1'b1)   dffe9a[1:1] <= combine_port[1:1];
	// synopsys translate_off
	initial
		dffe9a[2:2] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe9a[2:2] <= 1'b0;
		else if  (wire_dffe9a_ena[2:2] == 1'b1)   dffe9a[2:2] <= combine_port[2:2];
	// synopsys translate_off
	initial
		dffe9a[3:3] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe9a[3:3] <= 1'b0;
		else if  (wire_dffe9a_ena[3:3] == 1'b1)   dffe9a[3:3] <= combine_port[3:3];
	// synopsys translate_off
	initial
		dffe9a[4:4] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe9a[4:4] <= 1'b0;
		else if  (wire_dffe9a_ena[4:4] == 1'b1)   dffe9a[4:4] <= combine_port[4:4];
	// synopsys translate_off
	initial
		dffe9a[5:5] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe9a[5:5] <= 1'b0;
		else if  (wire_dffe9a_ena[5:5] == 1'b1)   dffe9a[5:5] <= combine_port[5:5];
	// synopsys translate_off
	initial
		dffe9a[6:6] = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) dffe9a[6:6] <= 1'b0;
		else if  (wire_dffe9a_ena[6:6] == 1'b1)   dffe9a[6:6] <= combine_port[6:6];
	assign
		wire_dffe9a_ena = {7{(idle & (write_param | read_param))}};
	// synopsys translate_off
	initial
		idle_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) idle_state <= {1{1'b1}};
		else  idle_state <= ((((((idle & (~ read_param)) & (~ write_param)) | write_wait) | (read_data & width_counter_all_done)) | (read_post & width_counter_all_done)) | power_up);
	// synopsys translate_off
	initial
		idle_write_wait = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) idle_write_wait <= 1'b0;
		else  idle_write_wait <= (((((((idle & (~ read_param)) & (~ write_param)) | write_wait) | (read_data & width_counter_all_done)) | (read_post & width_counter_all_done)) | power_up) & write_load);
	// synopsys translate_off
	initial
		read_address_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) read_address_state <= 1'b0;
		else if  (wire_read_address_state_ena == 1'b1)   read_address_state <= (((read_param | write_param) & ((param[2] & (~ param[1])) & (~ param[0]))) & (~ (~ idle)));
	assign
		wire_read_address_state_ena = (read_param | write_param);
	// synopsys translate_off
	initial
		read_data_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) read_data_state <= 1'b0;
		else  read_data_state <= (((read_init_counter & bit_counter_param_start_match) | (read_pre_data & bit_counter_param_start_match)) | ((read_data & (~ width_counter_param_width_match)) & (~ width_counter_all_done)));
	// synopsys translate_off
	initial
		read_init_counter_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) read_init_counter_state <= 1'b0;
		else  read_init_counter_state <= rsource_update_done;
	// synopsys translate_off
	initial
		read_init_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) read_init_state <= 1'b0;
		else  read_init_state <= (idle & read_param);
	// synopsys translate_off
	initial
		read_post_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) read_post_state <= 1'b0;
		else  read_post_state <= (((read_data & width_counter_param_width_match) & (~ width_counter_all_done)) | (read_post & (~ width_counter_all_done)));
	// synopsys translate_off
	initial
		read_pre_data_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) read_pre_data_state <= 1'b0;
		else  read_pre_data_state <= ((read_init_counter & (~ bit_counter_param_start_match)) | (read_pre_data & (~ bit_counter_param_start_match)));
	// synopsys translate_off
	initial
		read_source_update_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) read_source_update_state <= 1'b0;
		else  read_source_update_state <= ((read_init | read_source_update) & (~ rsource_update_done));
	// synopsys translate_off
	initial
		write_data_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) write_data_state <= 1'b0;
		else  write_data_state <= (((write_init_counter & bit_counter_param_start_match) | (write_pre_data & bit_counter_param_start_match)) | ((write_data & (~ width_counter_param_width_match)) & (~ bit_counter_all_done)));
	// synopsys translate_off
	initial
		write_init_counter_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) write_init_counter_state <= 1'b0;
		else  write_init_counter_state <= wsource_update_done;
	// synopsys translate_off
	initial
		write_init_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) write_init_state <= 1'b0;
		else  write_init_state <= (idle & write_param);
	// synopsys translate_off
	initial
		write_load_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) write_load_state <= 1'b0;
		else  write_load_state <= ((write_data & bit_counter_all_done) | (write_post_data & bit_counter_all_done));
	// synopsys translate_off
	initial
		write_post_data_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) write_post_data_state <= 1'b0;
		else  write_post_data_state <= (((write_data & width_counter_param_width_match) & (~ bit_counter_all_done)) | (write_post_data & (~ bit_counter_all_done)));
	// synopsys translate_off
	initial
		write_pre_data_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) write_pre_data_state <= 1'b0;
		else  write_pre_data_state <= ((write_init_counter & (~ bit_counter_param_start_match)) | (write_pre_data & (~ bit_counter_param_start_match)));
	// synopsys translate_off
	initial
		write_source_update_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) write_source_update_state <= 1'b0;
		else  write_source_update_state <= ((write_init | write_source_update) & (~ wsource_update_done));
	// synopsys translate_off
	initial
		write_wait_state = 0;
	// synopsys translate_on
	always @ ( posedge clock or  posedge reset)
		if (reset == 1'b1) write_wait_state <= 1'b0;
		else  write_wait_state <= write_load;
	lpm_counter   cntr5
	( 
	.aclr(reset),
	.clock(clock),
	.cnt_en(bit_counter_enable),
	.cout(),
	.eq(),
	.q(wire_cntr5_q),
	.sclr(bit_counter_clear)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.aload(1'b0),
	.aset(1'b0),
	.cin(1'b1),
	.clk_en(1'b1),
	.data({6{1'b0}}),
	.sload(1'b0),
	.sset(1'b0),
	.updown(1'b1)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	);
	defparam
		cntr5.lpm_direction = "UP",
		cntr5.lpm_port_updown = "PORT_UNUSED",
		cntr5.lpm_width = 6,
		cntr5.lpm_type = "lpm_counter";
	lpm_counter   cntr6
	( 
	.aclr(reset),
	.clock(clock),
	.cnt_en(width_counter_enable),
	.cout(),
	.eq(),
	.q(wire_cntr6_q),
	.sclr(width_counter_clear)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.aload(1'b0),
	.aset(1'b0),
	.cin(1'b1),
	.clk_en(1'b1),
	.data({5{1'b0}}),
	.sload(1'b0),
	.sset(1'b0),
	.updown(1'b1)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	);
	defparam
		cntr6.lpm_direction = "UP",
		cntr6.lpm_port_updown = "PORT_UNUSED",
		cntr6.lpm_width = 5,
		cntr6.lpm_type = "lpm_counter";
	cycloneive_rublock   sd4
	( 
	.captnupdt(rublock_captnupdt),
	.clk(rublock_clock),
	.rconfig(rublock_reconfig),
	.regin(rublock_regin),
	.regout(wire_sd4_regout),
	.rsttimer(reset_timer),
	.shiftnld(rublock_shiftnld));
	assign
		bit_counter_all_done = (((((wire_cntr5_q[0] & (~ wire_cntr5_q[1])) & (~ wire_cntr5_q[2])) & wire_cntr5_q[3]) & (~ wire_cntr5_q[4])) & wire_cntr5_q[5]),
		bit_counter_clear = (rsource_update_done | wsource_update_done),
		bit_counter_enable = (((((((((rsource_update_done | wsource_update_done) | read_init_counter) | write_init_counter) | read_pre_data) | write_pre_data) | read_data) | write_data) | read_post) | write_post_data),
		bit_counter_param_start = start_bit_decoder_out,
		bit_counter_param_start_match = ((((((~ w53w[0]) & (~ w53w[1])) & (~ w53w[2])) & (~ w53w[3])) & (~ w53w[4])) & (~ w53w[5])),
		busy = (~ idle),
		combine_port = {read_param, write_param, read_source, param},
		data_out = {((read_address & dffe7a[26]) | ((~ read_address) & dffe7a[28])), ((read_address & dffe7a[25]) | ((~ read_address) & dffe7a[27])), ((read_address & dffe7a[24]) | ((~ read_address) & dffe7a[26])), ((read_address & dffe7a[23]) | ((~ read_address) & dffe7a[25])), ((read_address & dffe7a[22]) | ((~ read_address) & dffe7a[24])), ((read_address & dffe7a[21]) | ((~ read_address) & dffe7a[23])), ((read_address & dffe7a[20]) | ((~ read_address) & dffe7a[22])), ((read_address & dffe7a[19]) | ((~ read_address) & dffe7a[21])), ((read_address & dffe7a[18]) | ((~ read_address) & dffe7a[20])), ((read_address & dffe7a[17]) | ((~ read_address) & dffe7a[19])), ((read_address & dffe7a[16]) | ((~ read_address) & dffe7a[18])), ((read_address & dffe7a[15]) | ((~ read_address) & dffe7a[17])), ((read_address & dffe7a[14]) | ((~ read_address) & dffe7a[16])), ((read_address & dffe7a[13]) | ((~ read_address) & dffe7a[15])), ((read_address & dffe7a[12]) | ((~ read_address) & dffe7a[14])), ((read_address & dffe7a[11]) | ((~ read_address) & dffe7a[13])), ((read_address & dffe7a[10]) | ((~ read_address) & dffe7a[12])), ((read_address & dffe7a[9]) | ((~ read_address) & dffe7a[11])), ((read_address & dffe7a[8]) | ((~ read_address) & dffe7a[10])), ((read_address & dffe7a[7]) | ((~ read_address) & dffe7a[9])), ((read_address & dffe7a[6]) | ((~ read_address) & dffe7a[8])), ((read_address & dffe7a[5]) | ((~ read_address) & dffe7a[7])), ((read_address & dffe7a[4]) | ((~ read_address) & dffe7a[6])), ((read_address & dffe7a[3]) | ((~ read_address) & dffe7a[5])), ((read_address & dffe7a[2]) | ((~ read_address) & dffe7a[4])), ((read_address & dffe7a[1]) | ((~ read_address) & dffe7a[3])), ((read_address & dffe7a[0]) | ((~ read_address) & dffe7a[2])), ((~ read_address) & dffe7a[1]), ((~ read_address) & dffe7a[0])},
		global_gnd = 1'b0,
		global_vcc = 1'b1,
		idle = idle_state,
		param_decoder_param_latch = dffe9a,
		param_decoder_select = {(((((((~ param_decoder_param_latch[0]) & param_decoder_param_latch[1]) & param_decoder_param_latch[2]) & param_decoder_param_latch[3]) & param_decoder_param_latch[4]) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & (~ param_decoder_param_latch[1])) & param_decoder_param_latch[2]) & param_decoder_param_latch[3]) & param_decoder_param_latch[4]) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), ((((((param_decoder_param_latch[0] & param_decoder_param_latch[1]) & (~ param_decoder_param_latch[2])) & param_decoder_param_latch[3]) & param_decoder_param_latch[4]) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & param_decoder_param_latch[1]) & (~ param_decoder_param_latch[2])) & param_decoder_param_latch[3]) & param_decoder_param_latch[4]) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), ((((((param_decoder_param_latch[0] & (~ param_decoder_param_latch[1])) & (~ param_decoder_param_latch[2])) & param_decoder_param_latch[3]) & param_decoder_param_latch[4]) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & param_decoder_param_latch[1]) & param_decoder_param_latch[2]) & (~ param_decoder_param_latch[3])) & (~ param_decoder_param_latch[4])) & param_decoder_param_latch[5]) & (~ param_decoder_param_latch[6])), (((((((~ param_decoder_param_latch[0]) & (~ param_decoder_param_latch[1])) & param_decoder_param_latch[2]) & (~ param_decoder_param_latch[3])) & (~ param_decoder_param_latch[4])) & param_decoder_param_latch[5]) & (~ param_decoder_param_latch[6])), ((((((param_decoder_param_latch[0] & param_decoder_param_latch[1]) & (~ param_decoder_param_latch[2])) & (~ param_decoder_param_latch[3])) & (~ param_decoder_param_latch[4])) & param_decoder_param_latch[5]) & (~ param_decoder_param_latch[6])), (((((((~ param_decoder_param_latch[0]) & param_decoder_param_latch[1]) & (~ param_decoder_param_latch[2]
)) & (~ param_decoder_param_latch[3])) & (~ param_decoder_param_latch[4])) & param_decoder_param_latch[5]) & (~ param_decoder_param_latch[6])), ((((((param_decoder_param_latch[0] & (~ param_decoder_param_latch[1])) & (~ param_decoder_param_latch[2])) & (~ param_decoder_param_latch[3])) & (~ param_decoder_param_latch[4])) & param_decoder_param_latch[5]) & (~ param_decoder_param_latch[6])), ((((((param_decoder_param_latch[0] & param_decoder_param_latch[1]) & param_decoder_param_latch[2]) & (~ param_decoder_param_latch[3])) & param_decoder_param_latch[4]) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & (~ param_decoder_param_latch[1])) & param_decoder_param_latch[2]) & (~ param_decoder_param_latch[3])) & param_decoder_param_latch[4]) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & (~ param_decoder_param_latch[1])) & (~ param_decoder_param_latch[2])) & (~ param_decoder_param_latch[3])) & param_decoder_param_latch[4]) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), ((((((param_decoder_param_latch[0] & param_decoder_param_latch[1]) & param_decoder_param_latch[2]) & param_decoder_param_latch[3]) & (~ param_decoder_param_latch[4])) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & (~ param_decoder_param_latch[1])) & param_decoder_param_latch[2]) & param_decoder_param_latch[3]) & (~ param_decoder_param_latch[4])) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & (~ param_decoder_param_latch[1])) & (~ param_decoder_param_latch[2])) & param_decoder_param_latch[3]) & (~ param_decoder_param_latch[4])) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & (~ param_decoder_param_latch[1])) & param_decoder_param_latch[2]) & (~ param_decoder_param_latch[3])) & param_decoder_param_latch[4]) & (~ param_decoder_param_latch[5]
)) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & (~ param_decoder_param_latch[1])) & (~ param_decoder_param_latch[2])) & (~ param_decoder_param_latch[3])) & param_decoder_param_latch[4]) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), ((((((param_decoder_param_latch[0] & param_decoder_param_latch[1]) & (~ param_decoder_param_latch[2])) & param_decoder_param_latch[3]) & (~ param_decoder_param_latch[4])) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & param_decoder_param_latch[1]) & (~ param_decoder_param_latch[2])) & param_decoder_param_latch[3]) & (~ param_decoder_param_latch[4])) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & (~ param_decoder_param_latch[1])) & (~ param_decoder_param_latch[2])) & param_decoder_param_latch[3]) & (~ param_decoder_param_latch[4])) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & (~ param_decoder_param_latch[1])) & param_decoder_param_latch[2]) & (~ param_decoder_param_latch[3])) & (~ param_decoder_param_latch[4])) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6]), (((((((~ param_decoder_param_latch[0]) & (~ param_decoder_param_latch[1])) & (~ param_decoder_param_latch[2])) & (~ param_decoder_param_latch[3])) & (~ param_decoder_param_latch[4])) & (~ param_decoder_param_latch[5])) & param_decoder_param_latch[6])},
		power_up = (((((((((((((((~ idle) & (~ read_init)) & (~ read_source_update)) & (~ read_init_counter)) & (~ read_pre_data)) & (~ read_data)) & (~ read_post)) & (~ write_init)) & (~ write_init_counter)) & (~ write_source_update)) & (~ write_pre_data)) & (~ write_data)) & (~ write_post_data)) & (~ write_load)) & (~ write_wait)),
		read_address = read_address_state,
		read_data = read_data_state,
		read_init = read_init_state,
		read_init_counter = read_init_counter_state,
		read_post = read_post_state,
		read_pre_data = read_pre_data_state,
		read_source_update = read_source_update_state,
		rsource_load = (idle & (write_param | read_param)),
		rsource_parallel_in = {((w4w[1] & read_param) | write_param), ((w4w[0] & read_param) | write_param)},
		rsource_serial_out = dffe1a0[0:0],
		rsource_shift_enable = (read_source_update | write_source_update),
		rsource_state_par_ini = {read_param, {2{global_gnd}}},
		rsource_update_done = dffe2a0[0:0],
		rublock_captnupdt = (~ write_load),
		rublock_clock = (~ (clock | idle_write_wait)),
		rublock_reconfig = rublock_reconfig_st,
		rublock_reconfig_st = (idle & reconfig),
		rublock_regin = (((((rublock_regout_reg & (~ select_shift_nloop)) & (~ read_source_update)) & (~ write_source_update)) | (((shift_reg_serial_out & select_shift_nloop) & (~ read_source_update)) & (~ write_source_update))) | ((read_source_update | write_source_update) & rsource_serial_out)),
		rublock_regout = wire_sd4_regout,
		rublock_regout_reg = dffe8,
		rublock_shiftnld = (((((((read_pre_data | write_pre_data) | read_data) | write_data) | read_post) | write_post_data) | read_source_update) | write_source_update),
		select_shift_nloop = ((read_data & (~ width_counter_param_width_match)) | (write_data & (~ width_counter_param_width_match))),
		shift_reg_clear = rsource_update_done,
		shift_reg_load_enable = (idle & write_param),
		shift_reg_serial_in = (rublock_regout_reg & select_shift_nloop),
		shift_reg_serial_out = dffe7a[0:0],
		shift_reg_shift_enable = (((read_data | write_data) | read_post) | write_post_data),
		start_bit_decoder_out = (((((((((((((((((((((({1'b0, {4{start_bit_decoder_param_select[0]}}, 1'b0} | {6{1'b0}}) | {1'b0, {4{start_bit_decoder_param_select[2]}}, 1'b0}) | {6{1'b0}}) | {1'b0, {3{start_bit_decoder_param_select[4]}}, 1'b0, start_bit_decoder_param_select[4]}) | {1'b0, {4{start_bit_decoder_param_select[5]}}, 1'b0}) | {6{1'b0}}) | {1'b0, {2{start_bit_decoder_param_select[7]}}, {3{1'b0}}}) | {6{1'b0}}) | {1'b0, {2{start_bit_decoder_param_select[9]}}, 1'b0, start_bit_decoder_param_select[9], 1'b0}) | {1'b0, {2{start_bit_decoder_param_select[10]}}, {3{1'b0}}}) | {6{1'b0}}) | {1'b0, {2{start_bit_decoder_param_select[12]}}, 1'b0, start_bit_decoder_param_select[12], 1'b0}) | {start_bit_decoder_param_select[13], {2{1'b0}}, start_bit_decoder_param_select[13], 1'b0, start_bit_decoder_param_select[13]}) | {6{1'b0}}) | {start_bit_decoder_param_select[15], {3{1'b0}}, {2{start_bit_decoder_param_select[15]}}}) | {{2{1'b0}}, {2{start_bit_decoder_param_select[16]}}, {2{1'b0}}}) | {start_bit_decoder_param_select[17], {2{1'b0}}, start_bit_decoder_param_select[17], {2{1'b0}}}) | {start_bit_decoder_param_select[18], {2{1'b0}}, start_bit_decoder_param_select[18], 1'b0, start_bit_decoder_param_select[18]}) | {6{1'b0}}) | {start_bit_decoder_param_select[20], {3{1'b0}}, {2{start_bit_decoder_param_select[20]}}}) | {{2{1'b0}}, {2{start_bit_decoder_param_select[21]}}, {2{1'b0}}}) | {start_bit_decoder_param_select[22], {2{1'b0}}, start_bit_decoder_param_select[22], {2{1'b0}}}),
		start_bit_decoder_param_select = param_decoder_select,
		w4w = read_source,
		w53w = (wire_cntr5_q ^ bit_counter_param_start),
		w83w = (wire_cntr6_q ^ width_counter_param_width),
		width_counter_all_done = (((((~ wire_cntr6_q[0]) & (~ wire_cntr6_q[1])) & wire_cntr6_q[2]) & wire_cntr6_q[3]) & wire_cntr6_q[4]),
		width_counter_clear = (rsource_update_done | wsource_update_done),
		width_counter_enable = ((read_data | write_data) | read_post),
		width_counter_param_width = width_decoder_out,
		width_counter_param_width_match = (((((~ w83w[0]) & (~ w83w[1])) & (~ w83w[2])) & (~ w83w[3])) & (~ w83w[4])),
		width_decoder_out = (((((((((((((((((((((({{3{1'b0}}, width_decoder_param_select[0], 1'b0} | {{2{width_decoder_param_select[1]}}, {3{1'b0}}}) | {{3{1'b0}}, width_decoder_param_select[2], 1'b0}) | {{3{width_decoder_param_select[3]}}, 1'b0, width_decoder_param_select[3]}) | {{4{1'b0}}, width_decoder_param_select[4]}) | {{3{1'b0}}, width_decoder_param_select[5], 1'b0}) | {{2{width_decoder_param_select[6]}}, {3{1'b0}}}) | {{3{1'b0}}, width_decoder_param_select[7], 1'b0}) | {{2{width_decoder_param_select[8]}}, {3{1'b0}}}) | {{2{1'b0}}, width_decoder_param_select[9], 1'b0, width_decoder_param_select[9]}) | {{3{1'b0}}, width_decoder_param_select[10], 1'b0}) | {{2{width_decoder_param_select[11]}}, {3{1'b0}}}) | {{2{1'b0}}, width_decoder_param_select[12], 1'b0, width_decoder_param_select[12]}) | {{4{1'b0}}, width_decoder_param_select[13]}) | {1'b0, {2{width_decoder_param_select[14]}}, {2{1'b0}}}) | {{4{1'b0}}, width_decoder_param_select[15]}) | {width_decoder_param_select[16], 1'b0, {2{width_decoder_param_select[16]}}, 1'b0}) | {{4{1'b0}}, width_decoder_param_select[17]}) | {{4{1'b0}}, width_decoder_param_select[18]}) | {1'b0, {2{width_decoder_param_select[19]}}, {2{1'b0}}}) | {{4{1'b0}}, width_decoder_param_select[20]}) | {width_decoder_param_select[21], 1'b0, {2{width_decoder_param_select[21]}}, 1'b0}) | {{4{1'b0}}, width_decoder_param_select[22]}),
		width_decoder_param_select = param_decoder_select,
		write_data = write_data_state,
		write_init = write_init_state,
		write_init_counter = write_init_counter_state,
		write_load = write_load_state,
		write_post_data = write_post_data_state,
		write_pre_data = write_pre_data_state,
		write_source_update = write_source_update_state,
		write_wait = write_wait_state,
		wsource_state_par_ini = {write_param, {2{global_gnd}}},
		wsource_update_done = dffe3a0[0:0];
endmodule //altera_remote_update_core
//VALID FILE
