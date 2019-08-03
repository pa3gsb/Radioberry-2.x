// Project			: Radioberry
//
// Module			: Top level design radioberry.v
//
// Target Devices	: Cyclone 10LP
//
// Tool 		 	: Quartus Prime Lite Edition v17.0.2
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description: 
//
//				Radioberry v2.0 SDR firmware code.
//
//
//	This firmware puts the radioberry into full duplex. 
//	The radioberry firmware is formed by this radioberry.rbf (verilog) and the radioberry (c-language) executable running at a rpi.
//	Radioberry.rbf firmware must be loaded into the FPGA by using the loadFPGA utility. 
// 	
//
// Johan Maas PA3GSB 
//
// Date:    25 July 2019
//
// Started on the day with a temperature record in Holland 40.7 celcius
//	
//------------------------------------------------------------------------------------------------------------------------------------------------------------

`include "timescale.v"

module radioberry(
input clk_76m8,
input ad9866_clk,
input [5:0]  ad9866_rx,
output [5:0] ad9866_tx,
input ad9866_rxsync,
input ad9866_rxclk,
output ad9866_txsync,
output ad9866_txquietn,
output ad9866_sclk,
output ad9866_sdio,
input ad9866_sdo,
output ad9866_sen_n,
output ad9866_rst_n,
output ad9866_mode,	
input spi_sck, 
input spi_mosi, 
output spi_miso, 
input [1:0] spi_ce,   
input pi_clk, 
input pi_clk2, 
output rx_samples, 
output [7:0] data,
input ptt_in, 
output ptt_out, 
output EER_PWM_out,
	
	(* useioff = 1 *) output  [1:0]   phy_tx,
	(* useioff = 1 *) output          phy_tx_en,
	(* useioff = 1 *) input   [1:0]   phy_rx,
	(* useioff = 1 *) input           phy_rx_dv,
	input           phy_clk,
	inout           phy_mdio,
	output          phy_mdc
);

reg   [9:0] PWM_min;					// sets minimum width of envelope PWM pulse
reg   [9:0] PWM_max;					// sets maximum width of envelope PWM pulse

logic clk_ad9866;
logic clk_ad9866_2x;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//     Ethernet
//------------------------------------------------------------------------------------------------------------------------------------------------------------

logic           clk_ctrl;

logic [5:0]     phy_clk_div = 5'b00000;
logic           phy_clk_div4;

logic [7:0]     phy_tx_data;
logic           phy_tx_valid;

logic [7:0]     phy_rx_data;
logic           phy_rx_valid;

always @(posedge phy_clk) begin
  phy_clk_div <= phy_clk_div + 2'b01;
end

assign phy_clk_div4 = phy_clk_div[1];

assign clk_ctrl = phy_clk_div[4]; //50.0/32


logic	run, run_sync, run_iosync;
logic           udp_tx_request;
logic [ 7:0]    udp_tx_data;
logic [10:0]    udp_tx_length;
logic           udp_tx_enable;
logic [15:0]    to_port;
logic           broadcast;
logic           udp_rx_active;
logic [ 7:0]    udp_rx_data;
logic           dst_unreachable;

network network_inst(
 // dhcp and mdio clock
  .clock_2_5MHz(clk_ctrl),
  .delay_start(~reset),
  
// upstream (rx-iq)
  .tx_clock(phy_clk_div4),
  .udp_tx_request(udp_tx_request),
  .udp_tx_length({5'h00,udp_tx_length}),
  .udp_tx_data(udp_tx_data),
  .udp_tx_enable(udp_tx_enable),
  .run(run_sync),
  .port_id(8'h00),
  
 // downstream (tx-iq)
  .rx_clock(phy_clk_div4),
  .to_port(to_port),
  .udp_rx_data(udp_rx_data),
  .udp_rx_active(udp_rx_active),
  .broadcast(broadcast),
  .dst_unreachable(dst_unreachable),
  
 // status and control
  .static_ip(IP),
  .local_mac(local_mac),
  .network_state_dhcp(network_state_dhcp),
  .network_state_fixedip(network_state_fixedip),
  .network_speed(),						// not used.
  
 // phy
  .phy_tx_data(phy_tx_data),
  .phy_tx_valid(phy_tx_valid),
  .phy_rx_data(phy_rx_data),
  .phy_rx_valid(phy_rx_valid),
    
  .PHY_MDIO(phy_mdio),
  .PHY_MDC(phy_mdc),

  .debug()
);

rmii_send rmii_send_i (
    .clk(phy_clk),
    .clk_div4(phy_clk_div4),
    .phy_tx_data(phy_tx_data),
    .phy_tx_valid(phy_tx_valid),
    .phy_tx(phy_tx),
    .phy_tx_en(phy_tx_en)
);

rmii_recv rmii_recv_i (
    .clk(phy_clk),
    .clk_div4(phy_clk_div4),
    .phy_rx_data(phy_rx_data),
    .phy_rx_valid(phy_rx_valid),
    .phy_rx(phy_rx),
    .phy_dv(phy_rx_dv)
);


//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         AD9866 Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------

assign ad9866_mode = 1'b1;				//FULLDUPLEX
assign ad9866_rst_n = ~reset;
assign ptt_out = ptt_in;
	 
// RX Path
logic   [11:0]    rx_data_assemble;
logic    [5:0]    rffe_ad9866_rx_d1, rffe_ad9866_rx_d2;
logic             rffe_ad9866_rxsync_d1;

always @(posedge clk_ad9866_2x) begin
  rffe_ad9866_rx_d1 <= ad9866_rx;
  rffe_ad9866_rx_d2 <= rffe_ad9866_rx_d1;
  rffe_ad9866_rxsync_d1 <= ad9866_rxsync;
  if (rffe_ad9866_rxsync_d1) rx_data_assemble <= {rffe_ad9866_rx_d2,rffe_ad9866_rx_d1};
end
	
// TX Path
logic   [11:0]    tx_data_d1;
logic             tx_sync;
logic             tx_en_d1;

always @(posedge clk_ad9866_2x) begin
  tx_en_d1 <= (ptt_in || vna_mode);
  tx_sync <= ~tx_sync;
  if (tx_en_d1) begin
    if (tx_sync) begin 
      tx_data_d1 <= DACDp;
      ad9866_tx <= tx_data_d1[5:0];
    end else begin
      ad9866_tx <= tx_data_d1[11:6];
    end
    ad9866_txsync <= tx_sync;
  end else begin
    ad9866_tx <= 6'h00;
    ad9866_txsync <= 1'b0;
  end
end

assign ad9866_txquietn = tx_en_d1; 	

wire ad9866_rx_rqst;
wire ad9866_tx_rqst;
reg [5:0] rx_gain;
reg [5:0] tx_gain;
reg vna_mode;

assign ad9866_rx_rqst = (!ptt_in && gain_update && ad9866_sen_n);
assign ad9866_tx_rqst = (ptt_in && gain_update && ad9866_sen_n);

ad9866 ad9866_inst(.reset(reset),.clk(clk_internal),.sclk(ad9866_sclk),.sdio(ad9866_sdio),.sdo(ad9866_sdo),.sen_n(ad9866_sen_n),.dataout(),.ext_tx_rqst(ad9866_tx_rqst),.tx_gain(tx_gain),.ext_rx_rqst(ad9866_rx_rqst),.rx_gain(rx_gain));


wire [11:0] DAC;

reg [11:0] DACDp;
always @ (negedge clk_ad9866) DACDp <= DAC;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         SPI Control dev 0
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//spi dev 0 ; used for settings.... rx/tx freq/gain speed; possible to make a command structure!!!!

reg[31:0] rx1_phase_word;
reg[31:0] rx2_phase_word;
reg[31:0] tx_phase_word;

wire [47:0] spi0_recv;
wire spi_done;
reg [47:0] spi_data;
reg pureSignal;
reg initDone;

reg [47:0] cmnd;
always @ (posedge spi_done) cmnd <= spi0_recv;

wire cmd_empty;
//using (small) fifo to bring commands through SPI bus to fast clock domain.
commandFIFO commandFIFO_inst (	.aclr(reset), 
								.wrclk(clk_internal), .wrreq(1'b1), .data(cmnd),	
								.rdclk(clk_ad9866), .rdreq(~cmd_empty),	.q(spi_data), .rdempty(cmd_empty));

localparam RX1 = 4'h1;
localparam RX2 = 4'h2;
localparam TX  = 4'h3;
localparam CONTROL = 4'h4;
								
always @ (posedge clk_ad9866)
begin
	if (reset) initDone <= 0; 
	if (~reset) begin
		 case (spi_data[47:44])
		  RX1: 	begin
					rx1_phase_word <= spi_data[31:0];
					rx1_speed <= spi_data[41:40];
					rx_gain <= ~spi_data[37:32];
					vna_mode <= spi_data[39];
					initDone <= 1; 
				end
		  RX2: 	begin
					rx2_phase_word <= spi_data[31:0];
					rx2_speed <= spi_data[41:40];  
				end
		  TX: 	begin
					tx_phase_word <= spi_data[31:0];
					tx_gain <= spi_data[37:32];  
				end
		  CONTROL: 
				begin
					pureSignal <= spi_data[32];
					PWM_min <= spi_data[9:0];
					PWM_max <= spi_data[25:16];
				end
		  default: 
				begin
					vna_mode <= 1'b1;
					rx1_phase_word <= 32'd0;
					rx1_speed <= 2'd0;
					rx_gain <= 6'd0;
					rx2_phase_word <= 32'd0;
					rx2_speed <= 2'd0;  
					tx_phase_word <= 32'd0;
					tx_gain <= 6'd0;
					pureSignal <= 0;
					PWM_min <= 10'd0;
					PWM_max <= 10'd1023;
				end
		endcase
	end
end

spi_slave spi_slave_rx_inst(.rstb(!reset),.ten(1'b1),.tdata(48'b0),.mlb(1'b1),.ss(spi_ce[0]),.sck(spi_sck),.sdin(spi_mosi), .sdout(spi_miso),.done(spi_done),.rdata(spi0_recv));

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         SPI Control dev 1
//------------------------------------------------------------------------------------------------------------------------------------------------------------
// spi dev 1 ; used for transmitting data ; always 48K
// sending back the filled tx buffer size!
wire [63:0] spi1_recv;
reg [63:0] tx_iq;
wire spi1_done;

always @ (posedge spi1_done) tx_iq <= spi1_recv[63:0];
		
spi_slave #(.WIDTH(64)) spi_slave_rx2_inst(.rstb(!reset),.ten(1'b1),.tdata({37'b0, tx_EER_rd_length, 5'b0,  tx_rd_length}),.mlb(1'b1),.ss(spi_ce[1]),.sck(spi_sck),.sdin(spi_mosi), .sdout(spi_miso),.done(spi1_done),.rdata(spi1_recv));


//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         Decimation Rate Control common
//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Decimation rates
localparam RATE48 = 6'd40;
localparam RATE96  =  RATE48  >> 1;
localparam RATE192 =  RATE96  >> 1;
localparam RATE384 =  RATE192 >> 1;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         Decimation Rate Control rx1
//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Decimation rates

reg [1:0] rx1_speed;	// selected decimation rate in external program,
reg [5:0] rx1_rate;

always @ (rx1_speed)
 begin 
	  case (rx1_speed)
	  0: rx1_rate <= RATE48;     
	  1: rx1_rate <= RATE96;     
	  2: rx1_rate <= RATE192;     
	  3: rx1_rate <= RATE384;           
	  default: rx1_rate <= RATE48;        
	  endcase
 end 

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         Decimation Rate Control rx2
//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Decimation rates

reg [1:0] rx2_speed;	// selected decimation rate in external program,
reg [5:0] rx2_rate;

always @ (rx2_speed)
 begin 
	  case (rx2_speed)
	  0: rx2_rate <= RATE48;     
	  1: rx2_rate <= RATE96;     
	  2: rx2_rate <= RATE192;     
	  3: rx2_rate <= RATE384;           
	  default: rx2_rate <= RATE48;        
	  endcase
 end 

//------------------------------------------------------------------------------
//                           Software Reset Handler
//------------------------------------------------------------------------------
wire reset;
reset_handler reset_handler_inst(.clock(clk_internal), .reset(reset));

//------------------------------------------------------------------------------
//                           Pipeline for adc fanout
//------------------------------------------------------------------------------
reg [11:0] adcpipe [0:1];

always @ (posedge clk_ad9866) begin
    adcpipe[0] <= rx_data_assemble;
    adcpipe[1] <= rx_data_assemble;
end


//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                        Receiver module rx1 
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire	[23:0] rx_I;
wire	[23:0] rx_Q;
wire	rx_strobe;

localparam CICRATE = 6'd05;

receiver #(.CICRATE(CICRATE)) 
		receiver_rx_inst(	
						.clock(clk_ad9866),
						.clock_2x(clk_ad9866_2x),
						.rate(rx1_rate), 
						.frequency(rx1_phase_word),
						.out_strobe(rx_strobe),
						.in_data(adcpipe[0]),
						.out_data_I(rx_I),
						.out_data_Q(rx_Q));

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                        Receiver module rx2 
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire	[23:0] rx2_I;
wire	[23:0] rx2_Q;
wire	rx2_strobe;

reg [11:0] pure_DACD; 
always @ (posedge clk_ad9866) pure_DACD <= DACDp;

wire [11:0] selected_input;
wire [31:0] selected_phase_word;
assign selected_input = (ptt_in & pureSignal) ?  pure_DACD : adcpipe[1];												
assign selected_phase_word = (ptt_in & pureSignal) ? tx_phase_word : rx2_phase_word; 	

receiver #(.CICRATE(CICRATE)) 
		receiver_rx2_inst(	
						.clock(clk_ad9866),
						.clock_2x(clk_ad9866_2x),
						.rate(rx2_rate), 
						.frequency(selected_phase_word), 
						.out_strobe(rx2_strobe),
						.in_data(selected_input),			
						.out_data_I(rx2_I),
						.out_data_Q(rx2_Q));
						
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          rx1FIFO Handler (IQ Samples) rx
//------------------------------------------------------------------------------------------------------------------------------------------------------------
reg [47:0] rxDataFromFIFO;
wire [10:0] rx_wr_length;
assign rx_samples = (rx_wr_length > 11'd63) ? 1'b1: 1'b0; 

rxFIFO rx1_FIFO_inst(	.aclr(reset),
							.wrclk(clk_ad9866),.data({rx_I, rx_Q}),.wrreq(rx_strobe), .wrusedw(rx_wr_length), .wrfull(),  
							.rdclk(rdreq),.q(rxDataFromFIFO ),.rdreq(1'b1));	

wire rdreq;
wire [7:0] rx1data_mux;
ddr_mux ddr_mux_inst1(.clk(pi_clk), .reset(reset | ~initDone), .rd_req(rdreq), .in_data(rxDataFromFIFO), .out_data(rx1data_mux));							

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          rx2FIFO Handler (IQ Samples) rx
//------------------------------------------------------------------------------------------------------------------------------------------------------------
reg [47:0] rx2DataFromFIFO;
wire [10:0] rx2_wr_length;
assign rx2_samples = (rx2_wr_length > 11'd63) ? 1'b1: 1'b0; 

rxFIFO rx2_FIFO_inst(	.aclr(reset),
						.wrclk(clk_ad9866),.data({rx2_I, rx2_Q}),.wrreq(rx2_strobe), .wrusedw(rx2_wr_length), .wrfull(),  
						.rdclk(rdreq2),.q(rx2DataFromFIFO),.rdreq(1'b1));					
							
wire rdreq2;	
wire [7:0] rx2data_mux;						
ddr_mux ddr_mux_inst2(.clk(pi_clk2), .reset(reset | ~initDone), .rd_req(rdreq2), .in_data(rx2DataFromFIFO), .out_data(rx2data_mux));

reg select;
always @(posedge pi_clk or posedge pi_clk2)
begin
	if (pi_clk == 1) select <= 0; else select <=1;
end
assign data = (select == 0) ? rx1data_mux : rx2data_mux; 

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          txFIFO Handler ( IQ-Transmit)
//------------------------------------------------------------------------------------------------------------------------------------------------------------
reg [10:0]  tx_rd_length; //return to c-firmware part via SPI when transmiting.
wire empty;

wire txreq = ptt_in ? 1'b1 : 1'b0 ;

txFIFO txFIFO_inst(	.aclr(reset), 
					.wrclk(~spi_ce[1]), .data(tx_iq), .wrreq(txreq), .wrfull (), .wrusedw (), 
					.rdclk(clk_ad9866), .q(txDataFromFIFO), .rdreq((txFIFOReadStrobe & ~empty)),  .rdempty(empty), .rdfull() , .rdusedw(tx_rd_length));
	
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                        Transmitter module
//------------------------------------------------------------------------------------------------------------------------------------------------------------							
wire [31:0] txDataFromFIFO;
wire txFIFOReadStrobe;
wire [31:0] txData;

transmitter transmitter_inst(	.reset(reset), .clk(clk_ad9866), .frequency(vna_mode ? rx1_phase_word : tx_phase_word), 
								.tsiq_data(txDataFromFIFO), .tsiq_read_strobe(txFIFOReadStrobe), .tsiq_valid(~empty), .CW_RF(CW_RF), 
								.out_data(DAC), .PTT(ptt_in), .CW_PTT(1'b0), .vna_mode(vna_mode),  .LED());


`ifdef EER

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                        EER module
//------------------------------------------------------------------------------------------------------------------------------------------------------------															
reg [10:0]  tx_EER_rd_length; //return to c-firmware part via SPI when transmiting.		
wire empty_EER;						
wire [31:0] txEERDataFromFIFO;	
			
								
txFIFO txFIFO_EER_inst(	.aclr(reset), 
						.wrclk(~spi_ce[1]), .data(tx_iq[63:32]), .wrreq(txreq), .wrfull (), .wrusedw (), 
						.rdclk(clk_ad9866), .q(txEERDataFromFIFO), .rdreq((req_EER & ~empty_EER)),  .rdempty(empty_EER), .rdfull() , .rdusedw(tx_EER_rd_length));
									
								
reg signed [15:0]C_EER_i;
reg signed [15:0]C_EER_q;

always @ (posedge clk_ad9866)
begin 
	if (req_EER & ~empty_EER )begin
		C_EER_i <= txEERDataFromFIFO[31:16]; 
		C_EER_q <= txEERDataFromFIFO[15:0];
	end 
end 								
								
wire req_EER;
wire [19:0] I_EER, Q_EER;
wire EER_enable;								
								
// Interpolate by 5 FIR for EER envelope generation - straight FIR, no CIC compensation.
// Note: Coefficients are scalled by 0.85 so that unity I&Q input give unity amplitude envelope signal.
FirInterp5_1025_EER fiEER (clk_ad9866, EER_enable, req_EER, C_EER_i, C_EER_q, I_EER, Q_EER); 								
								
//---------------------------------------------------------
//    Envelope generation for  EER Signal
//---------------------------------------------------------
// calculate the envelope of the SSB signal using SQRT(I^2 + Q^2)
wire [31:0] Isquare;
wire [31:0] Qsquare;
wire [32:0] sum;				// 32 bits + 32 bits requires 33 bit accumulator.
wire [15:0] envelope;

// use I&Q x 5 from EER iFIR output
square square_I (.clock(clk_ad9866), .dataa(I_EER[19:4]), .result(Isquare)); 
square square_Q (.clock(clk_ad9866), .dataa(Q_EER[19:4]), .result(Qsquare)); 
assign sum = Isquare + Qsquare;
sqroot sqroot_inst (.clk(clk_ad9866), .radical(sum[32:1]), .q(envelope)); 

//--------------------------------------------------------
// Generate 240kHz PWM signal from Envelope
//--------------------------------------------------------
reg  [9:0] ramp = 0; 
reg PWM = 0;

counter counter_inst (.clock(clk_envelope), .q(ramp));  // count to 1024 [10:0] = 240kHz, 640 [9:0] for 384kHz

wire [9:0] envelope_level = envelope[14:5] + (envelope[14:5] >>> 2)  + (envelope[14:5] >>> 3);  // Multiply by 1.25

always @ (posedge clk_envelope)
begin
  if ((ramp < PWM_min | envelope_level > ramp) && ramp < PWM_max) 
		PWM <= 1'b1;
  else PWM <= 1'b0;
end

assign EER_enable = (ramp == 10'd0 | ramp == 10'd1); 

assign EER_PWM_out = (ptt_in) ? PWM : 1'b0; 

`endif


wire clk_envelope;	
wire clk_internal;															
ad9866pll ad9866pll_inst (.inclk0(clk_76m8), .c0(clk_ad9866), .c1(clk_ad9866_2x), .c2(clk_envelope), .c3(clk_internal),  .locked());
								
//------------------------------------------------------------------------------
//                          Running...
//------------------------------------------------------------------------------
reg [26:0]counter;

always @(posedge clk_internal) 
begin
  if (reset)
    counter <= 26'b0;
  else
    counter <= counter + 1'b1;
end

wire gain_update;
assign gain_update = counter[22];

endmodule