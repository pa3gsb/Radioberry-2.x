//altasmi_parallel CBX_AUTO_BLACKBOX="ALL" CBX_SINGLE_OUTPUT_FILE="ON" DATA_WIDTH="STANDARD" DEVICE_FAMILY="Cyclone IV E" ENABLE_SIM="FALSE" EPCS_TYPE="EPCS16" FLASH_RSTPIN="FALSE" PAGE_SIZE=256 PORT_BULK_ERASE="PORT_USED" PORT_DIE_ERASE="PORT_UNUSED" PORT_EN4B_ADDR="PORT_UNUSED" PORT_EX4B_ADDR="PORT_UNUSED" PORT_FAST_READ="PORT_UNUSED" PORT_ILLEGAL_ERASE="PORT_USED" PORT_ILLEGAL_WRITE="PORT_USED" PORT_RDID_OUT="PORT_UNUSED" PORT_READ_ADDRESS="PORT_UNUSED" PORT_READ_DUMMYCLK="PORT_UNUSED" PORT_READ_RDID="PORT_UNUSED" PORT_READ_SID="PORT_USED" PORT_READ_STATUS="PORT_UNUSED" PORT_SECTOR_ERASE="PORT_USED" PORT_SECTOR_PROTECT="PORT_USED" PORT_SHIFT_BYTES="PORT_USED" PORT_WREN="PORT_USED" PORT_WRITE="PORT_USED" USE_ASMIBLOCK="ON" USE_EAB="ON" WRITE_DUMMY_CLK=0 addr bulk_erase busy clkin data_valid datain dataout epcs_id illegal_erase illegal_write rden read read_sid reset sector_erase sector_protect shift_bytes wren write INTENDED_DEVICE_FAMILY="Cyclone IV E" ALTERA_INTERNAL_OPTIONS=SUPPRESS_DA_RULE_INTERNAL=C106
//VERSION_BEGIN 18.1 cbx_a_gray2bin 2018:09:12:13:04:09:SJ cbx_a_graycounter 2018:09:12:13:04:09:SJ cbx_altasmi_parallel 2018:09:12:13:04:09:SJ cbx_altdpram 2018:09:12:13:04:09:SJ cbx_altera_counter 2018:09:12:13:04:09:SJ cbx_altera_syncram 2018:09:12:13:04:09:SJ cbx_altera_syncram_nd_impl 2018:09:12:13:04:09:SJ cbx_altsyncram 2018:09:12:13:04:09:SJ cbx_arriav 2018:09:12:13:04:09:SJ cbx_cyclone 2018:09:12:13:04:09:SJ cbx_cycloneii 2018:09:12:13:04:09:SJ cbx_fifo_common 2018:09:12:13:04:09:SJ cbx_lpm_add_sub 2018:09:12:13:04:09:SJ cbx_lpm_compare 2018:09:12:13:04:09:SJ cbx_lpm_counter 2018:09:12:13:04:09:SJ cbx_lpm_decode 2018:09:12:13:04:09:SJ cbx_lpm_mux 2018:09:12:13:04:09:SJ cbx_mgl 2018:09:12:14:15:07:SJ cbx_nadder 2018:09:12:13:04:09:SJ cbx_nightfury 2018:09:12:13:04:09:SJ cbx_scfifo 2018:09:12:13:04:09:SJ cbx_stratix 2018:09:12:13:04:09:SJ cbx_stratixii 2018:09:12:13:04:09:SJ cbx_stratixiii 2018:09:12:13:04:09:SJ cbx_stratixv 2018:09:12:13:04:09:SJ cbx_util_mgl 2018:09:12:13:04:09:SJ cbx_zippleback 2018:09:12:13:04:09:SJ  VERSION_END
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



//synthesis_resources = a_graycounter 6 cycloneive_asmiblock 1 lpm_compare 2 lpm_counter 2 lut 76 mux21 1 reg 130 
//synopsys translate_off
`timescale 1 ps / 1 ps
//synopsys translate_on
(* ALTERA_ATTRIBUTE = {"SUPPRESS_DA_RULE_INTERNAL=C106"} *)
module  asmi_asmi_parallel_0
	( 
	addr,
	bulk_erase,
	busy,
	clkin,
	data_valid,
	datain,
	dataout,
	epcs_id,
	illegal_erase,
	illegal_write,
	rden,
	read,
	read_sid,
	reset,
	sector_erase,
	sector_protect,
	shift_bytes,
	wren,
	write) /* synthesis synthesis_clearbox=1 */;
	input   [23:0]  addr;
	input   bulk_erase;
	output   busy;
	input   clkin;
	output   data_valid;
	input   [7:0]  datain;
	output   [7:0]  dataout;
	output   [7:0]  epcs_id;
	output   illegal_erase;
	output   illegal_write;
	input   rden;
	input   read;
	input   read_sid;
	input   reset;
	input   sector_erase;
	input   sector_protect;
	input   shift_bytes;
	input   wren;
	input   write;
`ifndef ALTERA_RESERVED_QIS
// synopsys translate_off
`endif
	tri0   bulk_erase;
	tri0   [7:0]  datain;
	tri0   read;
	tri0   read_sid;
	tri0   reset;
	tri0   sector_erase;
	tri0   sector_protect;
	tri0   shift_bytes;
	tri1   wren;
	tri0   write;
`ifndef ALTERA_RESERVED_QIS
// synopsys translate_on
`endif

	wire  [2:0]   wire_addbyte_cntr_q;
	wire  [2:0]   wire_gen_cntr_q;
	wire  [2:0]   wire_read_flag_status_cntr_q;
	wire  [1:0]   wire_spstage_cntr_q;
	wire  [1:0]   wire_stage_cntr_q;
	wire  [1:0]   wire_wrstage_cntr_q;
	wire  wire_sd2_data0out;
	reg	add_msb_reg;
	wire	wire_add_msb_reg_ena;
	wire	[23:0]	wire_addr_reg_d;
	reg	[23:0]	addr_reg;
	wire	[23:0]	wire_addr_reg_ena;
	wire	[7:0]	wire_asmi_opcode_reg_d;
	reg	[7:0]	asmi_opcode_reg;
	wire	[7:0]	wire_asmi_opcode_reg_ena;
	reg	buf_empty_reg;
	reg	bulk_erase_reg;
	wire	wire_bulk_erase_reg_ena;
	reg	busy_det_reg;
	reg	clr_read_reg;
	reg	clr_read_reg2;
	reg	clr_rstat_reg;
	reg	clr_secprot_reg;
	reg	clr_secprot_reg1;
	reg	clr_sid_reg;
	reg	clr_write_reg;
	reg	clr_write_reg2;
	reg	cnt_bfend_reg;
	reg	do_wrmemadd_reg;
	reg	dvalid_reg;
	wire	wire_dvalid_reg_ena;
	wire	wire_dvalid_reg_sclr;
	reg	dvalid_reg2;
	reg	end1_cyc_reg;
	reg	end1_cyc_reg2;
	reg	end_op_hdlyreg;
	reg	end_op_reg;
	reg	end_pgwrop_reg;
	wire	wire_end_pgwrop_reg_ena;
	reg	end_rbyte_reg;
	wire	wire_end_rbyte_reg_ena;
	wire	wire_end_rbyte_reg_sclr;
	reg	end_read_reg;
	reg	[7:0]	epcs_id_reg2;
	reg	ill_erase_reg;
	reg	ill_write_reg;
	reg	illegal_write_prot_reg;
	reg	max_cnt_reg;
	reg	maxcnt_shift_reg;
	reg	maxcnt_shift_reg2;
	reg	ncs_reg;
	wire	wire_ncs_reg_sclr;
	wire	[7:0]	wire_pgwrbuf_dataout_d;
	reg	[7:0]	pgwrbuf_dataout;
	wire	[7:0]	wire_pgwrbuf_dataout_ena;
	reg	read_bufdly_reg;
	wire	[7:0]	wire_read_data_reg_d;
	reg	[7:0]	read_data_reg;
	wire	[7:0]	wire_read_data_reg_ena;
	wire	[7:0]	wire_read_dout_reg_d;
	reg	[7:0]	read_dout_reg;
	wire	[7:0]	wire_read_dout_reg_ena;
	reg	read_reg;
	wire	wire_read_reg_ena;
	reg	read_sid_reg;
	wire	wire_read_sid_reg_ena;
	reg	sec_erase_reg;
	wire	wire_sec_erase_reg_ena;
	reg	sec_prot_reg;
	wire	wire_sec_prot_reg_ena;
	reg	shftpgwr_data_reg;
	reg	shift_op_reg;
	reg	sprot_rstat_reg;
	reg	stage2_reg;
	reg	stage3_dly_reg;
	reg	stage3_reg;
	reg	stage4_reg;
	reg	start_sppoll_reg;
	wire	wire_start_sppoll_reg_ena;
	reg	start_sppoll_reg2;
	reg	start_wrpoll_reg;
	wire	wire_start_wrpoll_reg_ena;
	reg	start_wrpoll_reg2;
	wire	[7:0]	wire_statreg_int_d;
	reg	[7:0]	statreg_int;
	wire	[7:0]	wire_statreg_int_ena;
	reg	streg_datain_reg;
	wire	wire_streg_datain_reg_ena;
	reg	write_prot_reg;
	wire	wire_write_prot_reg_ena;
	reg	write_reg;
	wire	wire_write_reg_ena;
	reg	write_rstat_reg;
	wire	[7:0]	wire_wrstat_dreg_d;
	reg	[7:0]	wrstat_dreg;
	wire	[7:0]	wire_wrstat_dreg_ena;
	wire  wire_cmpr4_aeb;
	wire  wire_cmpr5_aeb;
	wire  [8:0]   wire_pgwr_data_cntr_q;
	wire  [8:0]   wire_pgwr_read_cntr_q;
	wire	wire_mux211_dataout;
	wire  [7:0]   wire_scfifo3_q;
	wire  addr_overdie;
	wire  addr_overdie_pos;
	wire  [23:0]  addr_reg_overdie;
	wire  [7:0]  b4addr_opcode;
	wire  be_write_prot;
	wire  [7:0]  berase_opcode;
	wire  bp0_wire;
	wire  bp1_wire;
	wire  bp2_wire;
	wire  bp3_wire;
	wire  buf_empty;
	wire  bulk_erase_wire;
	wire  busy_wire;
	wire  clkin_wire;
	wire  clr_addmsb_wire;
	wire  clr_endrbyte_wire;
	wire  clr_read_wire;
	wire  clr_read_wire2;
	wire  clr_rstat_wire;
	wire  clr_secprot_wire;
	wire  clr_secprot_wire1;
	wire  clr_sid_wire;
	wire  clr_write_wire;
	wire  clr_write_wire2;
	wire  cnt_bfend_wire_in;
	wire  data0out_wire;
	wire  data_valid_wire;
	wire  [3:0]  datain_wire;
	wire  [3:0]  dataout_wire;
	wire  [7:0]  derase_opcode;
	wire  do_4baddr;
	wire  do_bulk_erase;
	wire  do_die_erase;
	wire  do_ex4baddr;
	wire  do_fast_read;
	wire  do_fread_epcq;
	wire  do_freadwrv_polling;
	wire  do_memadd;
	wire  do_polling;
	wire  do_read;
	wire  do_read_nonvolatile;
	wire  do_read_rdid;
	wire  do_read_sid;
	wire  do_read_stat;
	wire  do_read_volatile;
	wire  do_sec_erase;
	wire  do_sec_prot;
	wire  do_secprot_wren;
	wire  do_sprot_polling;
	wire  do_sprot_rstat;
	wire  do_wait_dummyclk;
	wire  do_wren;
	wire  do_write;
	wire  do_write_polling;
	wire  do_write_rstat;
	wire  do_write_volatile;
	wire  do_write_volatile_rstat;
	wire  do_write_volatile_wren;
	wire  do_write_wren;
	wire  dummy_read_buf;
	wire  end1_cyc_gen_cntr_wire;
	wire  end1_cyc_normal_in_wire;
	wire  end1_cyc_reg_in_wire;
	wire  end_add_cycle;
	wire  end_add_cycle_mux_datab_wire;
	wire  end_fast_read;
	wire  end_one_cyc_pos;
	wire  end_one_cycle;
	wire  end_op_wire;
	wire  end_operation;
	wire  end_ophdly;
	wire  end_pgwr_data;
	wire  end_read;
	wire  end_read_byte;
	wire  end_read_flag_status;
	wire  end_wrstage;
	wire  [7:0]  exb4addr_opcode;
	wire  [7:0]  fast_read_opcode;
	wire  fast_read_wire;
	wire  freadwrv_sdoin;
	wire  ill_erase_wire;
	wire  ill_write_wire;
	wire  illegal_erase_b4out_wire;
	wire  illegal_write_b4out_wire;
	wire  in_operation;
	wire  load_opcode;
	wire  [4:0]  mask_prot;
	wire  [4:0]  mask_prot_add;
	wire  [4:0]  mask_prot_check;
	wire  [4:0]  mask_prot_comp_ntb;
	wire  [4:0]  mask_prot_comp_tb;
	wire  memadd_sdoin;
	wire  ncs_reg_ena_wire;
	wire  not_busy;
	wire  oe_wire;
	wire  [8:0]  page_size_wire;
	wire  [8:0]  pagewr_buf_not_empty;
	wire  [7:0]  prot_wire;
	wire  rden_wire;
	wire  [7:0]  rdid_opcode;
	wire  [7:0]  rdummyclk_opcode;
	wire  reach_max_cnt;
	wire  read_buf;
	wire  read_bufdly;
	wire  [7:0]  read_data_reg_in_wire;
	wire  [7:0]  read_opcode;
	wire  read_rdid_wire;
	wire  read_sid_wire;
	wire  read_status_wire;
	wire  read_wire;
	wire  [7:0]  rflagstat_opcode;
	wire  [7:0]  rnvdummyclk_opcode;
	wire  [7:0]  rsid_opcode;
	wire  rsid_sdoin;
	wire  [7:0]  rstat_opcode;
	wire  scein_wire;
	wire  sdoin_wire;
	wire  sec_erase_wire;
	wire  sec_protect_wire;
	wire  [7:0]  secprot_opcode;
	wire  secprot_sdoin;
	wire  [7:0]  serase_opcode;
	wire  shift_bytes_wire;
	wire  shift_opcode;
	wire  shift_opdata;
	wire  shift_pgwr_data;
	wire  sid_load;
	wire  st_busy_wire;
	wire  stage2_wire;
	wire  stage3_wire;
	wire  stage4_wire;
	wire  start_frpoll;
	wire  start_poll;
	wire  start_sppoll;
	wire  start_wrpoll;
	wire  to_sdoin_wire;
	wire  [7:0]  wren_opcode;
	wire  wren_wire;
	wire  [7:0]  write_opcode;
	wire  write_prot_true;
	wire  write_sdoin;
	wire  write_wire;
	wire  [7:0]  wrvolatile_opcode;

	a_graycounter   addbyte_cntr
	( 
	.aclr(reset),
	.clk_en((((((wire_stage_cntr_q[1] & wire_stage_cntr_q[0]) & end_one_cyc_pos) & (((((((do_read_sid | do_write) | do_sec_erase) | do_die_erase) | do_read_rdid) | do_read) | do_fast_read) | do_read_nonvolatile)) | addr_overdie) | end_operation)),
	.clock((~ clkin_wire)),
	.q(wire_addbyte_cntr_q),
	.qbin(),
	.sclr((end_operation | addr_overdie))
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.cnt_en(1'b1),
	.updown(1'b1)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	);
	defparam
		addbyte_cntr.width = 3,
		addbyte_cntr.lpm_type = "a_graycounter";
	a_graycounter   gen_cntr
	( 
	.aclr(reset),
	.clk_en((((((in_operation & (~ end_ophdly)) & (~ clr_rstat_wire)) & (~ clr_sid_wire)) | do_wait_dummyclk) | addr_overdie)),
	.clock(clkin_wire),
	.q(wire_gen_cntr_q),
	.qbin(),
	.sclr(((end1_cyc_reg_in_wire | addr_overdie) | do_wait_dummyclk))
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.cnt_en(1'b1),
	.updown(1'b1)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	);
	defparam
		gen_cntr.width = 3,
		gen_cntr.lpm_type = "a_graycounter";
	a_graycounter   read_flag_status_cntr
	( 
	.aclr(reset),
	.clk_en(((((start_sppoll & end_operation) & (~ end_read_flag_status)) & (~ st_busy_wire)) | (clr_secprot_wire1 | ((start_sppoll & st_busy_wire) & stage3_wire)))),
	.clock(clkin_wire),
	.q(wire_read_flag_status_cntr_q),
	.qbin(),
	.sclr((clr_secprot_wire1 | ((start_sppoll & st_busy_wire) & stage3_wire)))
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.cnt_en(1'b1),
	.updown(1'b1)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	);
	defparam
		read_flag_status_cntr.width = 3,
		read_flag_status_cntr.lpm_type = "a_graycounter";
	a_graycounter   spstage_cntr
	( 
	.aclr(reset),
	.clk_en(((((do_sec_prot & end_operation) & (~ do_sprot_polling)) | (((do_sec_prot & end_operation) & do_sprot_polling) & end_read_flag_status)) | clr_secprot_wire1)),
	.clock((~ clkin_wire)),
	.q(wire_spstage_cntr_q),
	.qbin(),
	.sclr(clr_secprot_wire1)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.cnt_en(1'b1),
	.updown(1'b1)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	);
	defparam
		spstage_cntr.width = 2,
		spstage_cntr.lpm_type = "a_graycounter";
	a_graycounter   stage_cntr
	( 
	.aclr(reset),
	.clk_en(((((((((((((((in_operation & end_one_cycle) & (~ (stage3_wire & (~ end_add_cycle)))) & (~ (stage4_wire & (~ end_read)))) & (~ (stage4_wire & (~ end_fast_read)))) & (~ ((((do_write | do_sec_erase) | do_die_erase) | do_bulk_erase) & write_prot_true))) & (~ (do_write & (~ pagewr_buf_not_empty[8])))) & (~ (stage3_wire & st_busy_wire))) & (~ ((do_write & shift_pgwr_data) & (~ end_pgwr_data)))) & (~ (stage2_wire & do_wren))) & (~ ((((stage3_wire & (do_sec_erase | do_die_erase)) & (~ do_wren)) & (~ do_read_stat)) & (~ do_read_rdid)))) & (~ (stage3_wire & ((do_write_volatile | do_read_volatile) | do_read_nonvolatile)))) | ((stage3_wire & do_fast_read) & do_wait_dummyclk)) | addr_overdie) | end_ophdly)),
	.clock(clkin_wire),
	.q(wire_stage_cntr_q),
	.qbin(),
	.sclr((end_operation | addr_overdie))
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.cnt_en(1'b1),
	.updown(1'b1)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	);
	defparam
		stage_cntr.width = 2,
		stage_cntr.lpm_type = "a_graycounter";
	a_graycounter   wrstage_cntr
	( 
	.aclr(reset),
	.clk_en((((((((((do_write | do_sec_erase) | do_bulk_erase) | do_die_erase) & (~ write_prot_true)) | do_4baddr) | do_ex4baddr) & end_wrstage) & (~ st_busy_wire)) | clr_write_wire2)),
	.clock((~ clkin_wire)),
	.q(wire_wrstage_cntr_q),
	.qbin(),
	.sclr(clr_write_wire2)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.cnt_en(1'b1),
	.updown(1'b1)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	);
	defparam
		wrstage_cntr.width = 2,
		wrstage_cntr.lpm_type = "a_graycounter";
	cycloneive_asmiblock   sd2
	( 
	.data0out(wire_sd2_data0out),
	.dclkin(clkin_wire),
	.oe(oe_wire),
	.scein(scein_wire),
	.sdoin((sdoin_wire | datain_wire[0])));
	// synopsys translate_off
	initial
		add_msb_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) add_msb_reg <= 1'b0;
		else if  (wire_add_msb_reg_ena == 1'b1) 
			if (clr_addmsb_wire == 1'b1) add_msb_reg <= 1'b0;
			else  add_msb_reg <= addr_reg[23];
	assign
		wire_add_msb_reg_ena = ((((((((do_read | do_fast_read) | do_write) | do_sec_erase) | do_die_erase) & (~ (((do_write | do_sec_erase) | do_die_erase) & (~ do_memadd)))) & wire_stage_cntr_q[1]) & wire_stage_cntr_q[0]) | clr_addmsb_wire);
	// synopsys translate_off
	initial
		addr_reg[0:0] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[0:0] <= 1'b0;
		else if  (wire_addr_reg_ena[0:0] == 1'b1)   addr_reg[0:0] <= wire_addr_reg_d[0:0];
	// synopsys translate_off
	initial
		addr_reg[1:1] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[1:1] <= 1'b0;
		else if  (wire_addr_reg_ena[1:1] == 1'b1)   addr_reg[1:1] <= wire_addr_reg_d[1:1];
	// synopsys translate_off
	initial
		addr_reg[2:2] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[2:2] <= 1'b0;
		else if  (wire_addr_reg_ena[2:2] == 1'b1)   addr_reg[2:2] <= wire_addr_reg_d[2:2];
	// synopsys translate_off
	initial
		addr_reg[3:3] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[3:3] <= 1'b0;
		else if  (wire_addr_reg_ena[3:3] == 1'b1)   addr_reg[3:3] <= wire_addr_reg_d[3:3];
	// synopsys translate_off
	initial
		addr_reg[4:4] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[4:4] <= 1'b0;
		else if  (wire_addr_reg_ena[4:4] == 1'b1)   addr_reg[4:4] <= wire_addr_reg_d[4:4];
	// synopsys translate_off
	initial
		addr_reg[5:5] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[5:5] <= 1'b0;
		else if  (wire_addr_reg_ena[5:5] == 1'b1)   addr_reg[5:5] <= wire_addr_reg_d[5:5];
	// synopsys translate_off
	initial
		addr_reg[6:6] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[6:6] <= 1'b0;
		else if  (wire_addr_reg_ena[6:6] == 1'b1)   addr_reg[6:6] <= wire_addr_reg_d[6:6];
	// synopsys translate_off
	initial
		addr_reg[7:7] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[7:7] <= 1'b0;
		else if  (wire_addr_reg_ena[7:7] == 1'b1)   addr_reg[7:7] <= wire_addr_reg_d[7:7];
	// synopsys translate_off
	initial
		addr_reg[8:8] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[8:8] <= 1'b0;
		else if  (wire_addr_reg_ena[8:8] == 1'b1)   addr_reg[8:8] <= wire_addr_reg_d[8:8];
	// synopsys translate_off
	initial
		addr_reg[9:9] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[9:9] <= 1'b0;
		else if  (wire_addr_reg_ena[9:9] == 1'b1)   addr_reg[9:9] <= wire_addr_reg_d[9:9];
	// synopsys translate_off
	initial
		addr_reg[10:10] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[10:10] <= 1'b0;
		else if  (wire_addr_reg_ena[10:10] == 1'b1)   addr_reg[10:10] <= wire_addr_reg_d[10:10];
	// synopsys translate_off
	initial
		addr_reg[11:11] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[11:11] <= 1'b0;
		else if  (wire_addr_reg_ena[11:11] == 1'b1)   addr_reg[11:11] <= wire_addr_reg_d[11:11];
	// synopsys translate_off
	initial
		addr_reg[12:12] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[12:12] <= 1'b0;
		else if  (wire_addr_reg_ena[12:12] == 1'b1)   addr_reg[12:12] <= wire_addr_reg_d[12:12];
	// synopsys translate_off
	initial
		addr_reg[13:13] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[13:13] <= 1'b0;
		else if  (wire_addr_reg_ena[13:13] == 1'b1)   addr_reg[13:13] <= wire_addr_reg_d[13:13];
	// synopsys translate_off
	initial
		addr_reg[14:14] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[14:14] <= 1'b0;
		else if  (wire_addr_reg_ena[14:14] == 1'b1)   addr_reg[14:14] <= wire_addr_reg_d[14:14];
	// synopsys translate_off
	initial
		addr_reg[15:15] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[15:15] <= 1'b0;
		else if  (wire_addr_reg_ena[15:15] == 1'b1)   addr_reg[15:15] <= wire_addr_reg_d[15:15];
	// synopsys translate_off
	initial
		addr_reg[16:16] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[16:16] <= 1'b0;
		else if  (wire_addr_reg_ena[16:16] == 1'b1)   addr_reg[16:16] <= wire_addr_reg_d[16:16];
	// synopsys translate_off
	initial
		addr_reg[17:17] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[17:17] <= 1'b0;
		else if  (wire_addr_reg_ena[17:17] == 1'b1)   addr_reg[17:17] <= wire_addr_reg_d[17:17];
	// synopsys translate_off
	initial
		addr_reg[18:18] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[18:18] <= 1'b0;
		else if  (wire_addr_reg_ena[18:18] == 1'b1)   addr_reg[18:18] <= wire_addr_reg_d[18:18];
	// synopsys translate_off
	initial
		addr_reg[19:19] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[19:19] <= 1'b0;
		else if  (wire_addr_reg_ena[19:19] == 1'b1)   addr_reg[19:19] <= wire_addr_reg_d[19:19];
	// synopsys translate_off
	initial
		addr_reg[20:20] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[20:20] <= 1'b0;
		else if  (wire_addr_reg_ena[20:20] == 1'b1)   addr_reg[20:20] <= wire_addr_reg_d[20:20];
	// synopsys translate_off
	initial
		addr_reg[21:21] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[21:21] <= 1'b0;
		else if  (wire_addr_reg_ena[21:21] == 1'b1)   addr_reg[21:21] <= wire_addr_reg_d[21:21];
	// synopsys translate_off
	initial
		addr_reg[22:22] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[22:22] <= 1'b0;
		else if  (wire_addr_reg_ena[22:22] == 1'b1)   addr_reg[22:22] <= wire_addr_reg_d[22:22];
	// synopsys translate_off
	initial
		addr_reg[23:23] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) addr_reg[23:23] <= 1'b0;
		else if  (wire_addr_reg_ena[23:23] == 1'b1)   addr_reg[23:23] <= wire_addr_reg_d[23:23];
	assign
		wire_addr_reg_d = {((({23{not_busy}} & addr[23:1]) | ({23{stage3_wire}} & addr_reg[22:0])) | ({23{addr_overdie}} & addr_reg_overdie[23:1])), ((not_busy & addr[0]) | (addr_overdie & addr_reg_overdie[0]))};
	assign
		wire_addr_reg_ena = {24{((((rden_wire | wren_wire) & not_busy) | (stage4_wire & addr_overdie)) | (stage3_wire & ((((do_write | do_sec_erase) | do_die_erase) & do_memadd) | (do_read | do_fast_read))))}};
	// synopsys translate_off
	initial
		asmi_opcode_reg[0:0] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) asmi_opcode_reg[0:0] <= 1'b0;
		else if  (wire_asmi_opcode_reg_ena[0:0] == 1'b1)   asmi_opcode_reg[0:0] <= wire_asmi_opcode_reg_d[0:0];
	// synopsys translate_off
	initial
		asmi_opcode_reg[1:1] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) asmi_opcode_reg[1:1] <= 1'b0;
		else if  (wire_asmi_opcode_reg_ena[1:1] == 1'b1)   asmi_opcode_reg[1:1] <= wire_asmi_opcode_reg_d[1:1];
	// synopsys translate_off
	initial
		asmi_opcode_reg[2:2] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) asmi_opcode_reg[2:2] <= 1'b0;
		else if  (wire_asmi_opcode_reg_ena[2:2] == 1'b1)   asmi_opcode_reg[2:2] <= wire_asmi_opcode_reg_d[2:2];
	// synopsys translate_off
	initial
		asmi_opcode_reg[3:3] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) asmi_opcode_reg[3:3] <= 1'b0;
		else if  (wire_asmi_opcode_reg_ena[3:3] == 1'b1)   asmi_opcode_reg[3:3] <= wire_asmi_opcode_reg_d[3:3];
	// synopsys translate_off
	initial
		asmi_opcode_reg[4:4] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) asmi_opcode_reg[4:4] <= 1'b0;
		else if  (wire_asmi_opcode_reg_ena[4:4] == 1'b1)   asmi_opcode_reg[4:4] <= wire_asmi_opcode_reg_d[4:4];
	// synopsys translate_off
	initial
		asmi_opcode_reg[5:5] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) asmi_opcode_reg[5:5] <= 1'b0;
		else if  (wire_asmi_opcode_reg_ena[5:5] == 1'b1)   asmi_opcode_reg[5:5] <= wire_asmi_opcode_reg_d[5:5];
	// synopsys translate_off
	initial
		asmi_opcode_reg[6:6] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) asmi_opcode_reg[6:6] <= 1'b0;
		else if  (wire_asmi_opcode_reg_ena[6:6] == 1'b1)   asmi_opcode_reg[6:6] <= wire_asmi_opcode_reg_d[6:6];
	// synopsys translate_off
	initial
		asmi_opcode_reg[7:7] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) asmi_opcode_reg[7:7] <= 1'b0;
		else if  (wire_asmi_opcode_reg_ena[7:7] == 1'b1)   asmi_opcode_reg[7:7] <= wire_asmi_opcode_reg_d[7:7];
	assign
		wire_asmi_opcode_reg_d = {(((((((((((((((((({7{(load_opcode & do_read_sid)}} & rsid_opcode[7:1]) | ({7{(load_opcode & do_read_rdid)}} & rdid_opcode[7:1])) | ({7{(((load_opcode & do_sec_prot) & (~ do_wren)) & (~ do_read_stat))}} & secprot_opcode[7:1])) | ({7{(load_opcode & do_read)}} & read_opcode[7:1])) | ({7{(load_opcode & do_fast_read)}} & fast_read_opcode[7:1])) | ({7{((((load_opcode & do_read_volatile) & (~ do_write_volatile)) & (~ do_wren)) & (~ do_read_stat))}} & rdummyclk_opcode[7:1])) | ({7{((((load_opcode & do_write_volatile) & (~ do_read_volatile)) & (~ do_wren)) & (~ do_read_stat))}} & wrvolatile_opcode[7:1])) | ({7{(load_opcode & do_read_nonvolatile)}} & rnvdummyclk_opcode[7:1])) | ({7{(load_opcode & ((do_write & (~ do_read_stat)) & (~ do_wren)))}} & write_opcode[7:1])) | ({7{((load_opcode & do_read_stat) & (~ do_polling))}} & rstat_opcode[7:1])) | ({7{((load_opcode & do_read_stat) & do_polling)}} & rflagstat_opcode[7:1])) | ({7{(((load_opcode & do_sec_erase) & (~ do_wren)) & (~ do_read_stat))}} & serase_opcode[7:1])) | ({7{(((load_opcode & do_die_erase) & (~ do_wren)) & (~ do_read_stat))}} & derase_opcode[7:1])) | ({7{(((load_opcode & do_bulk_erase) & (~ do_wren)) & (~ do_read_stat))}} & berase_opcode[7:1])) | ({7{(load_opcode & do_wren)}} & wren_opcode[7:1])) | ({7{(load_opcode & ((do_4baddr & (~ do_read_stat)) & (~ do_wren)))}} & b4addr_opcode[7:1])) | ({7{(load_opcode & ((do_ex4baddr & (~ do_read_stat)) & (~ do_wren)))}} & exb4addr_opcode[7:1])) | ({7{shift_opcode}} & asmi_opcode_reg[6:0])), ((((((((((((((((((load_opcode & do_read_sid) & rsid_opcode[0]) | ((load_opcode & do_read_rdid) & rdid_opcode[0])) | ((((load_opcode & do_sec_prot) & (~ do_wren)) & (~ do_read_stat)) & secprot_opcode[0])) | ((load_opcode & do_read) & read_opcode[0])) | ((load_opcode & do_fast_read) & fast_read_opcode[0])) | (((((load_opcode & do_read_volatile) & (~ do_write_volatile)) & (~ do_wren)) & (~ do_read_stat)) & rdummyclk_opcode[0])) | (((((load_opcode & do_write_volatile) & (~ do_read_volatile)) & (~ do_wren)) & (~ do_read_stat
)) & wrvolatile_opcode[0])) | ((load_opcode & do_read_nonvolatile) & rnvdummyclk_opcode[0])) | ((load_opcode & ((do_write & (~ do_read_stat)) & (~ do_wren))) & write_opcode[0])) | (((load_opcode & do_read_stat) & (~ do_polling)) & rstat_opcode[0])) | (((load_opcode & do_read_stat) & do_polling) & rflagstat_opcode[0])) | ((((load_opcode & do_sec_erase) & (~ do_wren)) & (~ do_read_stat)) & serase_opcode[0])) | ((((load_opcode & do_die_erase) & (~ do_wren)) & (~ do_read_stat)) & derase_opcode[0])) | ((((load_opcode & do_bulk_erase) & (~ do_wren)) & (~ do_read_stat)) & berase_opcode[0])) | ((load_opcode & do_wren) & wren_opcode[0])) | ((load_opcode & ((do_4baddr & (~ do_read_stat)) & (~ do_wren))) & b4addr_opcode[0])) | ((load_opcode & ((do_ex4baddr & (~ do_read_stat)) & (~ do_wren))) & exb4addr_opcode[0]))};
	assign
		wire_asmi_opcode_reg_ena = {8{(load_opcode | shift_opcode)}};
	// synopsys translate_off
	initial
		buf_empty_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) buf_empty_reg <= 1'b0;
		else  buf_empty_reg <= wire_cmpr5_aeb;
	// synopsys translate_off
	initial
		bulk_erase_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) bulk_erase_reg <= 1'b0;
		else if  (wire_bulk_erase_reg_ena == 1'b1) 
			if (clr_write_wire == 1'b1) bulk_erase_reg <= 1'b0;
			else  bulk_erase_reg <= bulk_erase;
	assign
		wire_bulk_erase_reg_ena = (((~ busy_wire) & wren_wire) | clr_write_wire);
	// synopsys translate_off
	initial
		busy_det_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) busy_det_reg <= 1'b0;
		else  busy_det_reg <= (~ busy_wire);
	// synopsys translate_off
	initial
		clr_read_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) clr_read_reg <= 1'b0;
		else  clr_read_reg <= ((do_read_sid | do_sec_prot) | (end_operation & (do_read | do_fast_read)));
	// synopsys translate_off
	initial
		clr_read_reg2 = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) clr_read_reg2 <= 1'b0;
		else  clr_read_reg2 <= clr_read_reg;
	// synopsys translate_off
	initial
		clr_rstat_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) clr_rstat_reg <= 1'b0;
		else  clr_rstat_reg <= end_operation;
	// synopsys translate_off
	initial
		clr_secprot_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) clr_secprot_reg <= 1'b0;
		else  clr_secprot_reg <= (((wire_spstage_cntr_q[1] & wire_spstage_cntr_q[0]) & end_read_flag_status) | do_read_sid);
	// synopsys translate_off
	initial
		clr_secprot_reg1 = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) clr_secprot_reg1 <= 1'b0;
		else  clr_secprot_reg1 <= clr_secprot_reg;
	// synopsys translate_off
	initial
		clr_sid_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) clr_sid_reg <= 1'b0;
		else  clr_sid_reg <= end_ophdly;
	// synopsys translate_off
	initial
		clr_write_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) clr_write_reg <= 1'b0;
		else  clr_write_reg <= (((((((((((((((do_write | do_sec_erase) | do_bulk_erase) | do_die_erase) | do_4baddr) | do_ex4baddr) & wire_wrstage_cntr_q[1]) & (~ wire_wrstage_cntr_q[0])) & end_operation) | write_prot_true) | (do_write & (~ pagewr_buf_not_empty[8]))) | (((((((~ do_write) & (~ do_sec_erase)) & (~ do_bulk_erase)) & (~ do_die_erase)) & (~ do_4baddr)) & (~ do_ex4baddr)) & end_operation)) | do_read_sid) | do_sec_prot) | do_read) | do_fast_read);
	// synopsys translate_off
	initial
		clr_write_reg2 = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) clr_write_reg2 <= 1'b0;
		else  clr_write_reg2 <= clr_write_reg;
	// synopsys translate_off
	initial
		cnt_bfend_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) cnt_bfend_reg <= 1'b0;
		else  cnt_bfend_reg <= cnt_bfend_wire_in;
	// synopsys translate_off
	initial
		do_wrmemadd_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) do_wrmemadd_reg <= 1'b0;
		else  do_wrmemadd_reg <= (wire_wrstage_cntr_q[1] & wire_wrstage_cntr_q[0]);
	// synopsys translate_off
	initial
		dvalid_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) dvalid_reg <= 1'b0;
		else if  (wire_dvalid_reg_ena == 1'b1) 
			if (wire_dvalid_reg_sclr == 1'b1) dvalid_reg <= 1'b0;
			else  dvalid_reg <= (end_read_byte & end_one_cyc_pos);
	assign
		wire_dvalid_reg_ena = (do_read | do_fast_read),
		wire_dvalid_reg_sclr = (end_op_wire | end_operation);
	// synopsys translate_off
	initial
		dvalid_reg2 = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) dvalid_reg2 <= 1'b0;
		else  dvalid_reg2 <= dvalid_reg;
	// synopsys translate_off
	initial
		end1_cyc_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) end1_cyc_reg <= 1'b0;
		else  end1_cyc_reg <= end1_cyc_reg_in_wire;
	// synopsys translate_off
	initial
		end1_cyc_reg2 = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) end1_cyc_reg2 <= 1'b0;
		else  end1_cyc_reg2 <= end_one_cycle;
	// synopsys translate_off
	initial
		end_op_hdlyreg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) end_op_hdlyreg <= 1'b0;
		else  end_op_hdlyreg <= end_operation;
	// synopsys translate_off
	initial
		end_op_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) end_op_reg <= 1'b0;
		else  end_op_reg <= end_op_wire;
	// synopsys translate_off
	initial
		end_pgwrop_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) end_pgwrop_reg <= 1'b0;
		else if  (wire_end_pgwrop_reg_ena == 1'b1) 
			if (clr_write_wire == 1'b1) end_pgwrop_reg <= 1'b0;
			else  end_pgwrop_reg <= buf_empty;
	assign
		wire_end_pgwrop_reg_ena = (((cnt_bfend_reg & do_write) & shift_pgwr_data) | clr_write_wire);
	// synopsys translate_off
	initial
		end_rbyte_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) end_rbyte_reg <= 1'b0;
		else if  (wire_end_rbyte_reg_ena == 1'b1) 
			if (wire_end_rbyte_reg_sclr == 1'b1) end_rbyte_reg <= 1'b0;
			else  end_rbyte_reg <= (((do_read | do_fast_read) & wire_stage_cntr_q[1]) & (~ wire_stage_cntr_q[0]));
	assign
		wire_end_rbyte_reg_ena = (((wire_gen_cntr_q[2] & (~ wire_gen_cntr_q[1])) & wire_gen_cntr_q[0]) | clr_endrbyte_wire),
		wire_end_rbyte_reg_sclr = (clr_endrbyte_wire | addr_overdie);
	// synopsys translate_off
	initial
		end_read_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) end_read_reg <= 1'b0;
		else  end_read_reg <= ((((~ rden_wire) & (do_read | do_fast_read)) & data_valid_wire) & end_read_byte);
	// synopsys translate_off
	initial
		epcs_id_reg2 = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) epcs_id_reg2 <= 8'b0;
		else if  (sid_load == 1'b1)   epcs_id_reg2 <= {read_dout_reg[7:0]};
	// synopsys translate_off
	initial
		ill_erase_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) ill_erase_reg <= 1'b0;
		else  ill_erase_reg <= illegal_erase_b4out_wire;
	// synopsys translate_off
	initial
		ill_write_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) ill_write_reg <= 1'b0;
		else  ill_write_reg <= illegal_write_b4out_wire;
	// synopsys translate_off
	initial
		illegal_write_prot_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) illegal_write_prot_reg <= 1'b0;
		else  illegal_write_prot_reg <= do_write;
	// synopsys translate_off
	initial
		max_cnt_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) max_cnt_reg <= 1'b0;
		else  max_cnt_reg <= wire_cmpr4_aeb;
	// synopsys translate_off
	initial
		maxcnt_shift_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) maxcnt_shift_reg <= 1'b0;
		else  maxcnt_shift_reg <= (((reach_max_cnt & shift_bytes_wire) & wren_wire) & (~ do_write));
	// synopsys translate_off
	initial
		maxcnt_shift_reg2 = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) maxcnt_shift_reg2 <= 1'b0;
		else  maxcnt_shift_reg2 <= maxcnt_shift_reg;
	// synopsys translate_off
	initial
		ncs_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) ncs_reg <= 1'b0;
		else if  (ncs_reg_ena_wire == 1'b1) 
			if (wire_ncs_reg_sclr == 1'b1) ncs_reg <= 1'b0;
			else  ncs_reg <= 1'b1;
	assign
		wire_ncs_reg_sclr = (end_operation | addr_overdie_pos);
	// synopsys translate_off
	initial
		pgwrbuf_dataout[0:0] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) pgwrbuf_dataout[0:0] <= 1'b0;
		else if  (wire_pgwrbuf_dataout_ena[0:0] == 1'b1) 
			if (clr_write_wire == 1'b1) pgwrbuf_dataout[0:0] <= 1'b0;
			else  pgwrbuf_dataout[0:0] <= wire_pgwrbuf_dataout_d[0:0];
	// synopsys translate_off
	initial
		pgwrbuf_dataout[1:1] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) pgwrbuf_dataout[1:1] <= 1'b0;
		else if  (wire_pgwrbuf_dataout_ena[1:1] == 1'b1) 
			if (clr_write_wire == 1'b1) pgwrbuf_dataout[1:1] <= 1'b0;
			else  pgwrbuf_dataout[1:1] <= wire_pgwrbuf_dataout_d[1:1];
	// synopsys translate_off
	initial
		pgwrbuf_dataout[2:2] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) pgwrbuf_dataout[2:2] <= 1'b0;
		else if  (wire_pgwrbuf_dataout_ena[2:2] == 1'b1) 
			if (clr_write_wire == 1'b1) pgwrbuf_dataout[2:2] <= 1'b0;
			else  pgwrbuf_dataout[2:2] <= wire_pgwrbuf_dataout_d[2:2];
	// synopsys translate_off
	initial
		pgwrbuf_dataout[3:3] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) pgwrbuf_dataout[3:3] <= 1'b0;
		else if  (wire_pgwrbuf_dataout_ena[3:3] == 1'b1) 
			if (clr_write_wire == 1'b1) pgwrbuf_dataout[3:3] <= 1'b0;
			else  pgwrbuf_dataout[3:3] <= wire_pgwrbuf_dataout_d[3:3];
	// synopsys translate_off
	initial
		pgwrbuf_dataout[4:4] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) pgwrbuf_dataout[4:4] <= 1'b0;
		else if  (wire_pgwrbuf_dataout_ena[4:4] == 1'b1) 
			if (clr_write_wire == 1'b1) pgwrbuf_dataout[4:4] <= 1'b0;
			else  pgwrbuf_dataout[4:4] <= wire_pgwrbuf_dataout_d[4:4];
	// synopsys translate_off
	initial
		pgwrbuf_dataout[5:5] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) pgwrbuf_dataout[5:5] <= 1'b0;
		else if  (wire_pgwrbuf_dataout_ena[5:5] == 1'b1) 
			if (clr_write_wire == 1'b1) pgwrbuf_dataout[5:5] <= 1'b0;
			else  pgwrbuf_dataout[5:5] <= wire_pgwrbuf_dataout_d[5:5];
	// synopsys translate_off
	initial
		pgwrbuf_dataout[6:6] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) pgwrbuf_dataout[6:6] <= 1'b0;
		else if  (wire_pgwrbuf_dataout_ena[6:6] == 1'b1) 
			if (clr_write_wire == 1'b1) pgwrbuf_dataout[6:6] <= 1'b0;
			else  pgwrbuf_dataout[6:6] <= wire_pgwrbuf_dataout_d[6:6];
	// synopsys translate_off
	initial
		pgwrbuf_dataout[7:7] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) pgwrbuf_dataout[7:7] <= 1'b0;
		else if  (wire_pgwrbuf_dataout_ena[7:7] == 1'b1) 
			if (clr_write_wire == 1'b1) pgwrbuf_dataout[7:7] <= 1'b0;
			else  pgwrbuf_dataout[7:7] <= wire_pgwrbuf_dataout_d[7:7];
	assign
		wire_pgwrbuf_dataout_d = {(({7{read_bufdly}} & wire_scfifo3_q[7:1]) | ({7{(~ read_bufdly)}} & pgwrbuf_dataout[6:0])), (read_bufdly & wire_scfifo3_q[0])};
	assign
		wire_pgwrbuf_dataout_ena = {8{((read_bufdly | shift_pgwr_data) | clr_write_wire)}};
	// synopsys translate_off
	initial
		read_bufdly_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_bufdly_reg <= 1'b0;
		else  read_bufdly_reg <= read_buf;
	// synopsys translate_off
	initial
		read_data_reg[0:0] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_data_reg[0:0] <= 1'b0;
		else if  (wire_read_data_reg_ena[0:0] == 1'b1)   read_data_reg[0:0] <= wire_read_data_reg_d[0:0];
	// synopsys translate_off
	initial
		read_data_reg[1:1] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_data_reg[1:1] <= 1'b0;
		else if  (wire_read_data_reg_ena[1:1] == 1'b1)   read_data_reg[1:1] <= wire_read_data_reg_d[1:1];
	// synopsys translate_off
	initial
		read_data_reg[2:2] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_data_reg[2:2] <= 1'b0;
		else if  (wire_read_data_reg_ena[2:2] == 1'b1)   read_data_reg[2:2] <= wire_read_data_reg_d[2:2];
	// synopsys translate_off
	initial
		read_data_reg[3:3] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_data_reg[3:3] <= 1'b0;
		else if  (wire_read_data_reg_ena[3:3] == 1'b1)   read_data_reg[3:3] <= wire_read_data_reg_d[3:3];
	// synopsys translate_off
	initial
		read_data_reg[4:4] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_data_reg[4:4] <= 1'b0;
		else if  (wire_read_data_reg_ena[4:4] == 1'b1)   read_data_reg[4:4] <= wire_read_data_reg_d[4:4];
	// synopsys translate_off
	initial
		read_data_reg[5:5] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_data_reg[5:5] <= 1'b0;
		else if  (wire_read_data_reg_ena[5:5] == 1'b1)   read_data_reg[5:5] <= wire_read_data_reg_d[5:5];
	// synopsys translate_off
	initial
		read_data_reg[6:6] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_data_reg[6:6] <= 1'b0;
		else if  (wire_read_data_reg_ena[6:6] == 1'b1)   read_data_reg[6:6] <= wire_read_data_reg_d[6:6];
	// synopsys translate_off
	initial
		read_data_reg[7:7] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_data_reg[7:7] <= 1'b0;
		else if  (wire_read_data_reg_ena[7:7] == 1'b1)   read_data_reg[7:7] <= wire_read_data_reg_d[7:7];
	assign
		wire_read_data_reg_d = {read_data_reg_in_wire[7:0]};
	assign
		wire_read_data_reg_ena = {8{(((((do_read | do_fast_read) & wire_stage_cntr_q[1]) & (~ wire_stage_cntr_q[0])) & end_one_cyc_pos) & end_read_byte)}};
	// synopsys translate_off
	initial
		read_dout_reg[0:0] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_dout_reg[0:0] <= 1'b0;
		else if  (wire_read_dout_reg_ena[0:0] == 1'b1)   read_dout_reg[0:0] <= wire_read_dout_reg_d[0:0];
	// synopsys translate_off
	initial
		read_dout_reg[1:1] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_dout_reg[1:1] <= 1'b0;
		else if  (wire_read_dout_reg_ena[1:1] == 1'b1)   read_dout_reg[1:1] <= wire_read_dout_reg_d[1:1];
	// synopsys translate_off
	initial
		read_dout_reg[2:2] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_dout_reg[2:2] <= 1'b0;
		else if  (wire_read_dout_reg_ena[2:2] == 1'b1)   read_dout_reg[2:2] <= wire_read_dout_reg_d[2:2];
	// synopsys translate_off
	initial
		read_dout_reg[3:3] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_dout_reg[3:3] <= 1'b0;
		else if  (wire_read_dout_reg_ena[3:3] == 1'b1)   read_dout_reg[3:3] <= wire_read_dout_reg_d[3:3];
	// synopsys translate_off
	initial
		read_dout_reg[4:4] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_dout_reg[4:4] <= 1'b0;
		else if  (wire_read_dout_reg_ena[4:4] == 1'b1)   read_dout_reg[4:4] <= wire_read_dout_reg_d[4:4];
	// synopsys translate_off
	initial
		read_dout_reg[5:5] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_dout_reg[5:5] <= 1'b0;
		else if  (wire_read_dout_reg_ena[5:5] == 1'b1)   read_dout_reg[5:5] <= wire_read_dout_reg_d[5:5];
	// synopsys translate_off
	initial
		read_dout_reg[6:6] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_dout_reg[6:6] <= 1'b0;
		else if  (wire_read_dout_reg_ena[6:6] == 1'b1)   read_dout_reg[6:6] <= wire_read_dout_reg_d[6:6];
	// synopsys translate_off
	initial
		read_dout_reg[7:7] = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_dout_reg[7:7] <= 1'b0;
		else if  (wire_read_dout_reg_ena[7:7] == 1'b1)   read_dout_reg[7:7] <= wire_read_dout_reg_d[7:7];
	assign
		wire_read_dout_reg_d = {read_dout_reg[6:0], (data0out_wire | dataout_wire[1])};
	assign
		wire_read_dout_reg_ena = {8{((stage4_wire & ((do_read | do_fast_read) | do_read_sid)) | (stage3_wire & (((do_read_stat | do_read_rdid) | do_read_volatile) | do_read_nonvolatile)))}};
	// synopsys translate_off
	initial
		read_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_reg <= 1'b0;
		else if  (wire_read_reg_ena == 1'b1) 
			if (clr_read_wire == 1'b1) read_reg <= 1'b0;
			else  read_reg <= read;
	assign
		wire_read_reg_ena = (((~ busy_wire) & rden_wire) | clr_read_wire);
	// synopsys translate_off
	initial
		read_sid_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) read_sid_reg <= 1'b0;
		else if  (wire_read_sid_reg_ena == 1'b1) 
			if (clr_sid_wire == 1'b1) read_sid_reg <= 1'b0;
			else  read_sid_reg <= read_sid;
	assign
		wire_read_sid_reg_ena = ((~ busy_wire) | clr_sid_wire);
	// synopsys translate_off
	initial
		sec_erase_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) sec_erase_reg <= 1'b0;
		else if  (wire_sec_erase_reg_ena == 1'b1) 
			if (clr_write_wire == 1'b1) sec_erase_reg <= 1'b0;
			else  sec_erase_reg <= sector_erase;
	assign
		wire_sec_erase_reg_ena = (((~ busy_wire) & wren_wire) | clr_write_wire);
	// synopsys translate_off
	initial
		sec_prot_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) sec_prot_reg <= 1'b0;
		else if  (wire_sec_prot_reg_ena == 1'b1) 
			if (clr_secprot_wire == 1'b1) sec_prot_reg <= 1'b0;
			else  sec_prot_reg <= sector_protect;
	assign
		wire_sec_prot_reg_ena = (((~ busy_wire) & wren_wire) | clr_secprot_wire);
	// synopsys translate_off
	initial
		shftpgwr_data_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) shftpgwr_data_reg <= 1'b0;
		else
			if (end_operation == 1'b1) shftpgwr_data_reg <= 1'b0;
			else  shftpgwr_data_reg <= (((wire_stage_cntr_q[1] & (~ wire_stage_cntr_q[0])) & wire_wrstage_cntr_q[1]) & wire_wrstage_cntr_q[0]);
	// synopsys translate_off
	initial
		shift_op_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) shift_op_reg <= 1'b0;
		else  shift_op_reg <= ((~ wire_stage_cntr_q[1]) & wire_stage_cntr_q[0]);
	// synopsys translate_off
	initial
		sprot_rstat_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) sprot_rstat_reg <= 1'b0;
		else
			if (clr_secprot_wire == 1'b1) sprot_rstat_reg <= 1'b0;
			else  sprot_rstat_reg <= ((do_sec_prot & wire_spstage_cntr_q[1]) & wire_spstage_cntr_q[0]);
	// synopsys translate_off
	initial
		stage2_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) stage2_reg <= 1'b0;
		else  stage2_reg <= ((~ wire_stage_cntr_q[1]) & wire_stage_cntr_q[0]);
	// synopsys translate_off
	initial
		stage3_dly_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) stage3_dly_reg <= 1'b0;
		else  stage3_dly_reg <= (wire_stage_cntr_q[1] & wire_stage_cntr_q[0]);
	// synopsys translate_off
	initial
		stage3_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) stage3_reg <= 1'b0;
		else  stage3_reg <= (wire_stage_cntr_q[1] & wire_stage_cntr_q[0]);
	// synopsys translate_off
	initial
		stage4_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) stage4_reg <= 1'b0;
		else  stage4_reg <= (wire_stage_cntr_q[1] & (~ wire_stage_cntr_q[0]));
	// synopsys translate_off
	initial
		start_sppoll_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) start_sppoll_reg <= 1'b0;
		else if  (wire_start_sppoll_reg_ena == 1'b1) 
			if (clr_secprot_wire == 1'b1) start_sppoll_reg <= 1'b0;
			else  start_sppoll_reg <= (wire_stage_cntr_q[1] & wire_stage_cntr_q[0]);
	assign
		wire_start_sppoll_reg_ena = (((do_sprot_rstat & do_polling) & end_one_cycle) | clr_secprot_wire);
	// synopsys translate_off
	initial
		start_sppoll_reg2 = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) start_sppoll_reg2 <= 1'b0;
		else
			if (clr_secprot_wire == 1'b1) start_sppoll_reg2 <= 1'b0;
			else  start_sppoll_reg2 <= start_sppoll_reg;
	// synopsys translate_off
	initial
		start_wrpoll_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) start_wrpoll_reg <= 1'b0;
		else if  (wire_start_wrpoll_reg_ena == 1'b1) 
			if (clr_write_wire == 1'b1) start_wrpoll_reg <= 1'b0;
			else  start_wrpoll_reg <= (wire_stage_cntr_q[1] & wire_stage_cntr_q[0]);
	assign
		wire_start_wrpoll_reg_ena = (((do_write_rstat & do_polling) & end_one_cycle) | clr_write_wire);
	// synopsys translate_off
	initial
		start_wrpoll_reg2 = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) start_wrpoll_reg2 <= 1'b0;
		else
			if (clr_write_wire == 1'b1) start_wrpoll_reg2 <= 1'b0;
			else  start_wrpoll_reg2 <= start_wrpoll_reg;
	// synopsys translate_off
	initial
		statreg_int[0:0] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) statreg_int[0:0] <= 1'b0;
		else if  (wire_statreg_int_ena[0:0] == 1'b1) 
			if (clr_rstat_wire == 1'b1) statreg_int[0:0] <= 1'b0;
			else  statreg_int[0:0] <= wire_statreg_int_d[0:0];
	// synopsys translate_off
	initial
		statreg_int[1:1] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) statreg_int[1:1] <= 1'b0;
		else if  (wire_statreg_int_ena[1:1] == 1'b1) 
			if (clr_rstat_wire == 1'b1) statreg_int[1:1] <= 1'b0;
			else  statreg_int[1:1] <= wire_statreg_int_d[1:1];
	// synopsys translate_off
	initial
		statreg_int[2:2] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) statreg_int[2:2] <= 1'b0;
		else if  (wire_statreg_int_ena[2:2] == 1'b1) 
			if (clr_rstat_wire == 1'b1) statreg_int[2:2] <= 1'b0;
			else  statreg_int[2:2] <= wire_statreg_int_d[2:2];
	// synopsys translate_off
	initial
		statreg_int[3:3] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) statreg_int[3:3] <= 1'b0;
		else if  (wire_statreg_int_ena[3:3] == 1'b1) 
			if (clr_rstat_wire == 1'b1) statreg_int[3:3] <= 1'b0;
			else  statreg_int[3:3] <= wire_statreg_int_d[3:3];
	// synopsys translate_off
	initial
		statreg_int[4:4] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) statreg_int[4:4] <= 1'b0;
		else if  (wire_statreg_int_ena[4:4] == 1'b1) 
			if (clr_rstat_wire == 1'b1) statreg_int[4:4] <= 1'b0;
			else  statreg_int[4:4] <= wire_statreg_int_d[4:4];
	// synopsys translate_off
	initial
		statreg_int[5:5] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) statreg_int[5:5] <= 1'b0;
		else if  (wire_statreg_int_ena[5:5] == 1'b1) 
			if (clr_rstat_wire == 1'b1) statreg_int[5:5] <= 1'b0;
			else  statreg_int[5:5] <= wire_statreg_int_d[5:5];
	// synopsys translate_off
	initial
		statreg_int[6:6] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) statreg_int[6:6] <= 1'b0;
		else if  (wire_statreg_int_ena[6:6] == 1'b1) 
			if (clr_rstat_wire == 1'b1) statreg_int[6:6] <= 1'b0;
			else  statreg_int[6:6] <= wire_statreg_int_d[6:6];
	// synopsys translate_off
	initial
		statreg_int[7:7] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) statreg_int[7:7] <= 1'b0;
		else if  (wire_statreg_int_ena[7:7] == 1'b1) 
			if (clr_rstat_wire == 1'b1) statreg_int[7:7] <= 1'b0;
			else  statreg_int[7:7] <= wire_statreg_int_d[7:7];
	assign
		wire_statreg_int_d = {read_dout_reg[7:0]};
	assign
		wire_statreg_int_ena = {8{(((end_operation | ((do_polling & end_one_cyc_pos) & stage3_dly_reg)) & do_read_stat) | clr_rstat_wire)}};
	// synopsys translate_off
	initial
		streg_datain_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) streg_datain_reg <= 1'b0;
		else if  (wire_streg_datain_reg_ena == 1'b1) 
			if (end_operation == 1'b1) streg_datain_reg <= 1'b0;
			else  streg_datain_reg <= wrstat_dreg[7];
	assign
		wire_streg_datain_reg_ena = (((do_sec_prot & (~ wire_spstage_cntr_q[1])) & wire_spstage_cntr_q[0]) | end_operation);
	// synopsys translate_off
	initial
		write_prot_reg = 0;
	// synopsys translate_on
	always @ ( negedge clkin_wire or  posedge reset)
		if (reset == 1'b1) write_prot_reg <= 1'b0;
		else if  (wire_write_prot_reg_ena == 1'b1) 
			if (clr_write_wire == 1'b1) write_prot_reg <= 1'b0;
			else  write_prot_reg <= ((((do_write | do_sec_erase) & (~ mask_prot_comp_ntb[4])) & (~ prot_wire[0])) | be_write_prot);
	assign
		wire_write_prot_reg_ena = (((((((do_sec_erase | do_write) | do_bulk_erase) | do_die_erase) & (~ wire_wrstage_cntr_q[1])) & wire_wrstage_cntr_q[0]) & end_ophdly) | clr_write_wire);
	// synopsys translate_off
	initial
		write_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) write_reg <= 1'b0;
		else if  (wire_write_reg_ena == 1'b1) 
			if (clr_write_wire == 1'b1) write_reg <= 1'b0;
			else  write_reg <= write;
	assign
		wire_write_reg_ena = (((~ busy_wire) & wren_wire) | clr_write_wire);
	// synopsys translate_off
	initial
		write_rstat_reg = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) write_rstat_reg <= 1'b0;
		else
			if (clr_write_wire == 1'b1) write_rstat_reg <= 1'b0;
			else  write_rstat_reg <= ((((((do_write | do_sec_erase) | do_bulk_erase) | do_die_erase) | do_4baddr) | do_ex4baddr) & (((~ wire_wrstage_cntr_q[1]) & (~ wire_wrstage_cntr_q[0])) | (wire_wrstage_cntr_q[1] & (~ wire_wrstage_cntr_q[0]))));
	// synopsys translate_off
	initial
		wrstat_dreg[0:0] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) wrstat_dreg[0:0] <= 1'b0;
		else if  (wire_wrstat_dreg_ena[0:0] == 1'b1) 
			if (clr_secprot_wire == 1'b1) wrstat_dreg[0:0] <= 1'b0;
			else  wrstat_dreg[0:0] <= wire_wrstat_dreg_d[0:0];
	// synopsys translate_off
	initial
		wrstat_dreg[1:1] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) wrstat_dreg[1:1] <= 1'b0;
		else if  (wire_wrstat_dreg_ena[1:1] == 1'b1) 
			if (clr_secprot_wire == 1'b1) wrstat_dreg[1:1] <= 1'b0;
			else  wrstat_dreg[1:1] <= wire_wrstat_dreg_d[1:1];
	// synopsys translate_off
	initial
		wrstat_dreg[2:2] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) wrstat_dreg[2:2] <= 1'b0;
		else if  (wire_wrstat_dreg_ena[2:2] == 1'b1) 
			if (clr_secprot_wire == 1'b1) wrstat_dreg[2:2] <= 1'b0;
			else  wrstat_dreg[2:2] <= wire_wrstat_dreg_d[2:2];
	// synopsys translate_off
	initial
		wrstat_dreg[3:3] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) wrstat_dreg[3:3] <= 1'b0;
		else if  (wire_wrstat_dreg_ena[3:3] == 1'b1) 
			if (clr_secprot_wire == 1'b1) wrstat_dreg[3:3] <= 1'b0;
			else  wrstat_dreg[3:3] <= wire_wrstat_dreg_d[3:3];
	// synopsys translate_off
	initial
		wrstat_dreg[4:4] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) wrstat_dreg[4:4] <= 1'b0;
		else if  (wire_wrstat_dreg_ena[4:4] == 1'b1) 
			if (clr_secprot_wire == 1'b1) wrstat_dreg[4:4] <= 1'b0;
			else  wrstat_dreg[4:4] <= wire_wrstat_dreg_d[4:4];
	// synopsys translate_off
	initial
		wrstat_dreg[5:5] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) wrstat_dreg[5:5] <= 1'b0;
		else if  (wire_wrstat_dreg_ena[5:5] == 1'b1) 
			if (clr_secprot_wire == 1'b1) wrstat_dreg[5:5] <= 1'b0;
			else  wrstat_dreg[5:5] <= wire_wrstat_dreg_d[5:5];
	// synopsys translate_off
	initial
		wrstat_dreg[6:6] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) wrstat_dreg[6:6] <= 1'b0;
		else if  (wire_wrstat_dreg_ena[6:6] == 1'b1) 
			if (clr_secprot_wire == 1'b1) wrstat_dreg[6:6] <= 1'b0;
			else  wrstat_dreg[6:6] <= wire_wrstat_dreg_d[6:6];
	// synopsys translate_off
	initial
		wrstat_dreg[7:7] = 0;
	// synopsys translate_on
	always @ ( posedge clkin_wire or  posedge reset)
		if (reset == 1'b1) wrstat_dreg[7:7] <= 1'b0;
		else if  (wire_wrstat_dreg_ena[7:7] == 1'b1) 
			if (clr_secprot_wire == 1'b1) wrstat_dreg[7:7] <= 1'b0;
			else  wrstat_dreg[7:7] <= wire_wrstat_dreg_d[7:7];
	assign
		wire_wrstat_dreg_d = {(({7{not_busy}} & datain[7:1]) | ({7{stage3_wire}} & wrstat_dreg[6:0])), (not_busy & datain[0])};
	assign
		wire_wrstat_dreg_ena = {8{(((wren_wire & not_busy) | (((do_sec_prot & stage3_wire) & (~ wire_spstage_cntr_q[1])) & wire_spstage_cntr_q[0])) | clr_secprot_wire)}};
	lpm_compare   cmpr4
	( 
	.aeb(wire_cmpr4_aeb),
	.agb(),
	.ageb(),
	.alb(),
	.aleb(),
	.aneb(),
	.dataa({page_size_wire[8:0]}),
	.datab({wire_pgwr_data_cntr_q[8:0]})
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.aclr(1'b0),
	.clken(1'b1),
	.clock(1'b0)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	);
	defparam
		cmpr4.lpm_width = 9,
		cmpr4.lpm_type = "lpm_compare";
	lpm_compare   cmpr5
	( 
	.aeb(wire_cmpr5_aeb),
	.agb(),
	.ageb(),
	.alb(),
	.aleb(),
	.aneb(),
	.dataa({wire_pgwr_data_cntr_q[8:0]}),
	.datab({wire_pgwr_read_cntr_q[8:0]})
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.aclr(1'b0),
	.clken(1'b1),
	.clock(1'b0)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	);
	defparam
		cmpr5.lpm_width = 9,
		cmpr5.lpm_type = "lpm_compare";
	lpm_counter   pgwr_data_cntr
	( 
	.aclr(reset),
	.clk_en(((((shift_bytes_wire & wren_wire) & (~ reach_max_cnt)) & (~ do_write)) | clr_write_wire2)),
	.clock(clkin_wire),
	.cout(),
	.eq(),
	.q(wire_pgwr_data_cntr_q),
	.sclr(clr_write_wire2)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.aload(1'b0),
	.aset(1'b0),
	.cin(1'b1),
	.cnt_en(1'b1),
	.data({9{1'b0}}),
	.sload(1'b0),
	.sset(1'b0),
	.updown(1'b1)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	);
	defparam
		pgwr_data_cntr.lpm_direction = "UP",
		pgwr_data_cntr.lpm_port_updown = "PORT_UNUSED",
		pgwr_data_cntr.lpm_width = 9,
		pgwr_data_cntr.lpm_type = "lpm_counter";
	lpm_counter   pgwr_read_cntr
	( 
	.aclr(reset),
	.clk_en((read_buf | clr_write_wire2)),
	.clock(clkin_wire),
	.cout(),
	.eq(),
	.q(wire_pgwr_read_cntr_q),
	.sclr(clr_write_wire2)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_off
	`endif
	,
	.aload(1'b0),
	.aset(1'b0),
	.cin(1'b1),
	.cnt_en(1'b1),
	.data({9{1'b0}}),
	.sload(1'b0),
	.sset(1'b0),
	.updown(1'b1)
	`ifndef FORMAL_VERIFICATION
	// synopsys translate_on
	`endif
	);
	defparam
		pgwr_read_cntr.lpm_direction = "UP",
		pgwr_read_cntr.lpm_port_updown = "PORT_UNUSED",
		pgwr_read_cntr.lpm_width = 9,
		pgwr_read_cntr.lpm_type = "lpm_counter";
	assign		wire_mux211_dataout = (do_fast_read === 1'b1) ? end_add_cycle_mux_datab_wire : (wire_addbyte_cntr_q[1] & (~ wire_addbyte_cntr_q[0]));
	scfifo   scfifo3
	( 
	.aclr(reset),
	.almost_empty(),
	.almost_full(),
	.clock(clkin_wire),
	.data({datain[7:0]}),
	.eccstatus(),
	.empty(),
	.full(),
	.q(wire_scfifo3_q),
	.rdreq((read_buf | dummy_read_buf)),
	.sclr(clr_write_wire2),
	.usedw(),
	.wrreq(((shift_bytes_wire & wren_wire) & (~ do_write))));
	defparam
		scfifo3.lpm_numwords = 258,
		scfifo3.lpm_width = 8,
		scfifo3.lpm_widthu = 9,
		scfifo3.use_eab = "ON",
		scfifo3.lpm_type = "scfifo";
	assign
		addr_overdie = 1'b0,
		addr_overdie_pos = 1'b0,
		addr_reg_overdie = {24{1'b0}},
		b4addr_opcode = {8{1'b0}},
		be_write_prot = ((do_bulk_erase | do_die_erase) & (((bp3_wire | bp2_wire) | bp1_wire) | bp0_wire)),
		berase_opcode = 8'b11000111,
		bp0_wire = statreg_int[2],
		bp1_wire = statreg_int[3],
		bp2_wire = statreg_int[4],
		bp3_wire = statreg_int[6],
		buf_empty = buf_empty_reg,
		bulk_erase_wire = bulk_erase_reg,
		busy = busy_wire,
		busy_wire = ((((((((((((((do_read_rdid | do_read_sid) | do_read) | do_fast_read) | do_write) | do_sec_prot) | do_read_stat) | do_sec_erase) | do_bulk_erase) | do_die_erase) | do_4baddr) | do_read_volatile) | do_fread_epcq) | do_read_nonvolatile) | do_ex4baddr),
		clkin_wire = clkin,
		clr_addmsb_wire = (((((wire_stage_cntr_q[1] & (~ wire_stage_cntr_q[0])) & end_add_cycle) & end_one_cyc_pos) | (((~ do_read) & (~ do_fast_read)) & clr_write_wire2)) | ((((do_sec_erase | do_die_erase) & (~ do_wren)) & (~ do_read_stat)) & end_operation)),
		clr_endrbyte_wire = (((((do_read | do_fast_read) & (~ wire_gen_cntr_q[2])) & wire_gen_cntr_q[1]) & wire_gen_cntr_q[0]) | clr_read_wire2),
		clr_read_wire = clr_read_reg,
		clr_read_wire2 = clr_read_reg2,
		clr_rstat_wire = clr_rstat_reg,
		clr_secprot_wire = clr_secprot_reg,
		clr_secprot_wire1 = clr_secprot_reg1,
		clr_sid_wire = clr_sid_reg,
		clr_write_wire = clr_write_reg,
		clr_write_wire2 = clr_write_reg2,
		cnt_bfend_wire_in = ((wire_gen_cntr_q[2] & (~ wire_gen_cntr_q[1])) & wire_gen_cntr_q[0]),
		data0out_wire = wire_sd2_data0out,
		data_valid = data_valid_wire,
		data_valid_wire = dvalid_reg2,
		datain_wire = {{4{1'b0}}},
		dataout = {read_data_reg[7:0]},
		dataout_wire = {{4{1'b0}}},
		derase_opcode = {8{1'b0}},
		do_4baddr = 1'b0,
		do_bulk_erase = (((((((((~ do_read_nonvolatile) & (~ read_rdid_wire)) & (~ read_sid_wire)) & (~ sec_protect_wire)) & (~ (read_wire | fast_read_wire))) & (~ write_wire)) & (~ read_status_wire)) & (~ sec_erase_wire)) & bulk_erase_wire),
		do_die_erase = 1'b0,
		do_ex4baddr = 1'b0,
		do_fast_read = 1'b0,
		do_fread_epcq = 1'b0,
		do_freadwrv_polling = 1'b0,
		do_memadd = do_wrmemadd_reg,
		do_polling = ((do_write_polling | do_sprot_polling) | do_freadwrv_polling),
		do_read = ((((~ read_rdid_wire) & (~ read_sid_wire)) & (~ sec_protect_wire)) & read_wire),
		do_read_nonvolatile = 1'b0,
		do_read_rdid = 1'b0,
		do_read_sid = (((~ do_read_nonvolatile) & (~ read_rdid_wire)) & read_sid_wire),
		do_read_stat = ((((((((((~ do_read_nonvolatile) & (~ read_rdid_wire)) & (~ read_sid_wire)) & (~ sec_protect_wire)) & (~ (read_wire | fast_read_wire))) & (~ write_wire)) & read_status_wire) | do_write_rstat) | do_sprot_rstat) | do_write_volatile_rstat),
		do_read_volatile = 1'b0,
		do_sec_erase = ((((((((~ do_read_nonvolatile) & (~ read_rdid_wire)) & (~ read_sid_wire)) & (~ sec_protect_wire)) & (~ (read_wire | fast_read_wire))) & (~ write_wire)) & (~ read_status_wire)) & sec_erase_wire),
		do_sec_prot = ((((~ do_read_nonvolatile) & (~ read_rdid_wire)) & (~ read_sid_wire)) & sec_protect_wire),
		do_secprot_wren = ((do_sec_prot & (~ wire_spstage_cntr_q[1])) & (~ wire_spstage_cntr_q[0])),
		do_sprot_polling = ((do_sec_prot & wire_spstage_cntr_q[1]) & wire_spstage_cntr_q[0]),
		do_sprot_rstat = sprot_rstat_reg,
		do_wait_dummyclk = 1'b0,
		do_wren = ((do_write_wren | do_secprot_wren) | do_write_volatile_wren),
		do_write = ((((((~ do_read_nonvolatile) & (~ read_rdid_wire)) & (~ read_sid_wire)) & (~ sec_protect_wire)) & (~ (read_wire | fast_read_wire))) & write_wire),
		do_write_polling = (((((((do_write | do_sec_erase) | do_bulk_erase) | do_die_erase) | do_4baddr) | do_ex4baddr) & wire_wrstage_cntr_q[1]) & (~ wire_wrstage_cntr_q[0])),
		do_write_rstat = write_rstat_reg,
		do_write_volatile = 1'b0,
		do_write_volatile_rstat = 1'b0,
		do_write_volatile_wren = 1'b0,
		do_write_wren = ((~ wire_wrstage_cntr_q[1]) & wire_wrstage_cntr_q[0]),
		dummy_read_buf = maxcnt_shift_reg2,
		end1_cyc_gen_cntr_wire = ((wire_gen_cntr_q[2] & (~ wire_gen_cntr_q[1])) & (~ wire_gen_cntr_q[0])),
		end1_cyc_normal_in_wire = ((((((((((((~ wire_stage_cntr_q[0]) & (~ wire_stage_cntr_q[1])) & (~ wire_gen_cntr_q[2])) & wire_gen_cntr_q[1]) & wire_gen_cntr_q[0]) | ((~ ((~ wire_stage_cntr_q[0]) & (~ wire_stage_cntr_q[1]))) & end1_cyc_gen_cntr_wire)) | (do_read & end_read)) | (do_fast_read & end_fast_read)) | ((((do_write | do_sec_erase) | do_bulk_erase) | do_die_erase) & write_prot_true)) | (do_write & (~ pagewr_buf_not_empty[8]))) | ((do_read_stat & start_poll) & (~ st_busy_wire))) | (do_read_rdid & end_op_wire)),
		end1_cyc_reg_in_wire = end1_cyc_normal_in_wire,
		end_add_cycle = wire_mux211_dataout,
		end_add_cycle_mux_datab_wire = (wire_addbyte_cntr_q[2] & wire_addbyte_cntr_q[1]),
		end_fast_read = end_read_reg,
		end_one_cyc_pos = end1_cyc_reg2,
		end_one_cycle = end1_cyc_reg,
		end_op_wire = ((((((((((((wire_stage_cntr_q[1] & (~ wire_stage_cntr_q[0])) & ((((((~ do_read) & (~ do_fast_read)) & (~ (do_write & shift_pgwr_data))) & end_one_cycle) | (do_read & end_read)) | (do_fast_read & end_fast_read))) | ((((wire_stage_cntr_q[1] & wire_stage_cntr_q[0]) & do_read_stat) & end_one_cycle) & (~ do_polling))) | ((((((do_read_rdid & end_one_cyc_pos) & wire_stage_cntr_q[1]) & wire_stage_cntr_q[0]) & wire_addbyte_cntr_q[2]) & wire_addbyte_cntr_q[1]) & (~ wire_addbyte_cntr_q[0]))) | (((start_poll & do_read_stat) & do_polling) & (~ st_busy_wire))) | ((((~ wire_stage_cntr_q[1]) & wire_stage_cntr_q[0]) & (do_wren | (do_4baddr | (do_ex4baddr | (do_bulk_erase & (~ do_read_stat)))))) & end_one_cycle)) | ((((do_write | do_sec_erase) | do_bulk_erase) | do_die_erase) & write_prot_true)) | ((do_write & shift_pgwr_data) & end_pgwr_data)) | (do_write & (~ pagewr_buf_not_empty[8]))) | (((((wire_stage_cntr_q[1] & wire_stage_cntr_q[0]) & do_sec_prot) & (~ do_wren)) & (~ do_read_stat)) & end_one_cycle)) | ((((((wire_stage_cntr_q[1] & wire_stage_cntr_q[0]) & (do_sec_erase | do_die_erase)) & (~ do_wren)) & (~ do_read_stat)) & end_add_cycle) & end_one_cycle)) | (((wire_stage_cntr_q[1] & wire_stage_cntr_q[0]) & end_one_cycle) & ((do_write_volatile | do_read_volatile) | (do_read_nonvolatile & wire_addbyte_cntr_q[1])))),
		end_operation = end_op_reg,
		end_ophdly = end_op_hdlyreg,
		end_pgwr_data = end_pgwrop_reg,
		end_read = end_read_reg,
		end_read_byte = (end_rbyte_reg & (~ addr_overdie)),
		end_read_flag_status = wire_read_flag_status_cntr_q[0],
		end_wrstage = end_operation,
		epcs_id = {epcs_id_reg2[7:0]},
		exb4addr_opcode = {8{1'b0}},
		fast_read_opcode = {8{1'b0}},
		fast_read_wire = 1'b0,
		freadwrv_sdoin = 1'b0,
		ill_erase_wire = ill_erase_reg,
		ill_write_wire = ill_write_reg,
		illegal_erase = ill_erase_wire,
		illegal_erase_b4out_wire = (((do_sec_erase | do_bulk_erase) | do_die_erase) & write_prot_true),
		illegal_write = ill_write_wire,
		illegal_write_b4out_wire = ((do_write & write_prot_true) | (do_write & (~ pagewr_buf_not_empty[8]))),
		in_operation = busy_wire,
		load_opcode = (((((~ wire_stage_cntr_q[1]) & (~ wire_stage_cntr_q[0])) & (~ wire_gen_cntr_q[2])) & (~ wire_gen_cntr_q[1])) & wire_gen_cntr_q[0]),
		mask_prot = {((((prot_wire[1] | prot_wire[2]) | prot_wire[3]) | prot_wire[4]) | prot_wire[5]), (((prot_wire[1] | prot_wire[2]) | prot_wire[3]) | prot_wire[4]), ((prot_wire[1] | prot_wire[2]) | prot_wire[3]), (prot_wire[1] | prot_wire[2]), prot_wire[1]},
		mask_prot_add = {(mask_prot[4] & addr_reg[20]), (mask_prot[3] & addr_reg[19]), (mask_prot[2] & addr_reg[18]), (mask_prot[1] & addr_reg[17]), (mask_prot[0] & addr_reg[16])},
		mask_prot_check = {(mask_prot[4] ^ mask_prot_add[4]), (mask_prot[3] ^ mask_prot_add[3]), (mask_prot[2] ^ mask_prot_add[2]), (mask_prot[1] ^ mask_prot_add[1]), (mask_prot[0] ^ mask_prot_add[0])},
		mask_prot_comp_ntb = {(mask_prot_check[4] | mask_prot_comp_ntb[3]), (mask_prot_check[3] | mask_prot_comp_ntb[2]), (mask_prot_check[2] | mask_prot_comp_ntb[1]), (mask_prot_check[1] | mask_prot_comp_ntb[0]), mask_prot_check[0]},
		mask_prot_comp_tb = {(mask_prot_add[4] | mask_prot_comp_tb[3]), (mask_prot_add[3] | mask_prot_comp_tb[2]), (mask_prot_add[2] | mask_prot_comp_tb[1]), (mask_prot_add[1] | mask_prot_comp_tb[0]), mask_prot_add[0]},
		memadd_sdoin = add_msb_reg,
		ncs_reg_ena_wire = (((((~ wire_stage_cntr_q[1]) & wire_stage_cntr_q[0]) & end_one_cyc_pos) | addr_overdie_pos) | end_operation),
		not_busy = busy_det_reg,
		oe_wire = 1'b0,
		page_size_wire = 9'b100000000,
		pagewr_buf_not_empty = {(pagewr_buf_not_empty[7] | wire_pgwr_data_cntr_q[8]), (pagewr_buf_not_empty[6] | wire_pgwr_data_cntr_q[7]), (pagewr_buf_not_empty[5] | wire_pgwr_data_cntr_q[6]), (pagewr_buf_not_empty[4] | wire_pgwr_data_cntr_q[5]), (pagewr_buf_not_empty[3] | wire_pgwr_data_cntr_q[4]), (pagewr_buf_not_empty[2] | wire_pgwr_data_cntr_q[3]), (pagewr_buf_not_empty[1] | wire_pgwr_data_cntr_q[2]), (pagewr_buf_not_empty[0] | wire_pgwr_data_cntr_q[1]), wire_pgwr_data_cntr_q[0]},
		prot_wire = {((bp2_wire & bp1_wire) & bp0_wire), ((bp2_wire & bp1_wire) & (~ bp0_wire)), ((bp2_wire & (~ bp1_wire)) & bp0_wire), ((bp2_wire & (~ bp1_wire)) & (~ bp0_wire)), (((~ bp2_wire) & bp1_wire) & bp0_wire), (((~ bp2_wire) & bp1_wire) & (~ bp0_wire)), (((~ bp2_wire) & (~ bp1_wire)) & bp0_wire), (((~ bp2_wire) & (~ bp1_wire)) & (~ bp0_wire))},
		rden_wire = rden,
		rdid_opcode = {8{1'b0}},
		rdummyclk_opcode = {8{1'b0}},
		reach_max_cnt = max_cnt_reg,
		read_buf = (((((end_one_cycle & do_write) & (~ do_read_stat)) & (~ do_wren)) & ((wire_stage_cntr_q[1] & (~ wire_stage_cntr_q[0])) | (wire_addbyte_cntr_q[1] & (~ wire_addbyte_cntr_q[0])))) & (~ buf_empty)),
		read_bufdly = read_bufdly_reg,
		read_data_reg_in_wire = {read_dout_reg[7:0]},
		read_opcode = 8'b00000011,
		read_rdid_wire = 1'b0,
		read_sid_wire = read_sid_reg,
		read_status_wire = 1'b0,
		read_wire = read_reg,
		rflagstat_opcode = 8'b00000101,
		rnvdummyclk_opcode = {8{1'b0}},
		rsid_opcode = 8'b10101011,
		rsid_sdoin = (do_read_sid & stage3_wire),
		rstat_opcode = 8'b00000101,
		scein_wire = (~ ncs_reg),
		sdoin_wire = to_sdoin_wire,
		sec_erase_wire = sec_erase_reg,
		sec_protect_wire = sec_prot_reg,
		secprot_opcode = 8'b00000001,
		secprot_sdoin = (stage3_wire & streg_datain_reg),
		serase_opcode = 8'b11011000,
		shift_bytes_wire = shift_bytes,
		shift_opcode = shift_op_reg,
		shift_opdata = stage2_wire,
		shift_pgwr_data = shftpgwr_data_reg,
		sid_load = (end_ophdly & do_read_sid),
		st_busy_wire = statreg_int[0],
		stage2_wire = stage2_reg,
		stage3_wire = stage3_reg,
		stage4_wire = stage4_reg,
		start_frpoll = 1'b0,
		start_poll = ((start_wrpoll | start_sppoll) | start_frpoll),
		start_sppoll = start_sppoll_reg2,
		start_wrpoll = start_wrpoll_reg2,
		to_sdoin_wire = ((((((shift_opdata & asmi_opcode_reg[7]) | rsid_sdoin) | memadd_sdoin) | write_sdoin) | secprot_sdoin) | freadwrv_sdoin),
		wren_opcode = 8'b00000110,
		wren_wire = wren,
		write_opcode = 8'b00000010,
		write_prot_true = write_prot_reg,
		write_sdoin = ((((do_write & stage4_wire) & wire_wrstage_cntr_q[1]) & wire_wrstage_cntr_q[0]) & pgwrbuf_dataout[7]),
		write_wire = write_reg,
		wrvolatile_opcode = {8{1'b0}};
endmodule //asmi_asmi_parallel_0
//VALID FILE
