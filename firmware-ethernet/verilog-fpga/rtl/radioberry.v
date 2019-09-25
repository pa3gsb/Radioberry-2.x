// Project			: Radioberry
//
// Module			: Top level design radioberry.v
//
// Target Devices	: Cyclone 10LP
//
// Tool 		 	: Quartus Prime Lite Edition v18.1.0
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description: 
//
//				Radioberry v2.0 SDR gateware. 
//
//
// 	Implementing ethernet module to transfer the high data rate streams, like rx-iq stream, tx-iq stream from radioberry to rpi and vice versa.
//	The slow control streams, like rx/tx frequency, sample rates uses the SPI bus!
//
//
//	This firmware puts the radioberry into full duplex. 
//	The radioberry firmware is formed by this radioberry.rbf (verilog) gateware and radioberry (c-language) executable running at a rpi.
//	
// 	The c-language part makes it possible to implement different SDR programs used protocols like openhpsdr p-1 or p-2.
//
//	Radioberry.rbf firmware must be loaded into the FPGA by using the loadFPGA utility. 
// 	
//
// Johan Maas PA3GSB 
//
// Date:    25 July 2019
//
// Started on the day with a temperature record in Holland: 40.7 Celcius
//	
//------------------------------------------------------------------------------------------------------------------------------------------------------------

`include "timescale.v"

module radioberry(

input clk_76m8,

input [5:0]  ad9866_rx, output [5:0] ad9866_tx, input ad9866_rxsync, input ad9866_rxclk, output ad9866_txsync, output ad9866_txquietn, 

output ad9866_sclk, output ad9866_sdio, input ad9866_sdo, output ad9866_sen_n, output ad9866_rst_n, output ad9866_mode,	

input spi_sck, input spi_mosi, output spi_miso, input [1:0] spi_ce,
 
input ptt_in, output ptt_out, 

  // Ethernet PHY
  (* useioff = 1 *) output  [1:0]   phy_tx,
  (* useioff = 1 *) output          phy_tx_en,
  (* useioff = 1 *) input   [1:0]   phy_rx,
  (* useioff = 1 *) input           phy_rx_dv,

input           phy_clk,
inout           phy_mdio,
output          phy_mdc
);

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//     Parameters
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//localparam       IP = {8'd169,8'd254,8'd45,8'd222};
localparam       IP = {8'd0,8'd0,8'd0,8'd0};
localparam       MAC = {8'h00,8'h1c,8'hc0,8'ha2,8'h2d,8'hde};

logic [31:0] 	radioberry_ip;
logic 	run_radio;

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

logic [15:0]    reset_counter = 16'h0000;
always @(posedge clk_ctrl) begin
  if (~reset_counter[15]) reset_counter <= reset_counter + 16'h01;
end

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
	.delay_start(~reset_counter[15]),
  
// rxstream (rx-iq)
	.tx_clock(phy_clk_div4),
	.udp_tx_request(udp_tx_request),
	.udp_tx_length({5'h00,udp_tx_length}),
	.udp_tx_data(udp_tx_data),
	.udp_tx_enable(udp_tx_enable),
	.run(run_radio),		
	.port_id(8'h00),
  
// txstream (tx-iq)
	.rx_clock(phy_clk_div4),
	.to_port(to_port),
	.udp_rx_data(udp_rx_data),
	.udp_rx_active(udp_rx_active),
	.broadcast(broadcast),
	.dst_unreachable(dst_unreachable), 
  
// status and control
	.radioberry_ip(radioberry_ip),
	.static_ip(IP),
	.local_mac(MAC),
	.network_state_dhcp(network_state_dhcp),
	.network_state_fixedip(network_state_fixedip), 
	.network_speed(),					

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
//                         RX stream handler; get rx iq samples from fifo and add to udp packet.
//------------------------------------------------------------------------------------------------------------------------------------------------------------
rxstream rxstream_rx1(
						.clk(phy_clk_div4), 
						.run(run_radio),
						.have_ip(~(network_state_dhcp & network_state_fixedip)),
						.udp_tx_enable(udp_tx_enable), 
						.udp_tx_request(udp_tx_request), 
						.udp_tx_data(udp_tx_data), 
						.udp_tx_length(udp_tx_length),
						.rx_data(fifo_rx_data),
						.rx_request(rx_request),
						.rx_length(fifo_rx_length));

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         SPI Control dev 0
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//spi dev 0 ; used for settings.... rx/tx freq/gain/sample speed
reg[31:0] rx1_phase_word;
reg[31:0] rx2_phase_word;
reg[31:0] tx_phase_word;

wire [47:0] spi0_recv;
wire spi_done;
reg [47:0] spi_data;

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
	 case (spi_data[47:44])
	  RX1: 	begin
				rx1_phase_word <= spi_data[31:0];
				rx1_speed <= spi_data[41:40];
				rx_gain <= ~spi_data[37:32];
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
				run_radio <= spi_data[40:40];
			end
	  default: 
			begin
				rx1_phase_word <= 32'd0;
				rx1_speed <= 2'd0;
				rx_gain <= 6'd0;
				rx2_phase_word <= 32'd0;
				rx2_speed <= 2'd0;  
				tx_phase_word <= 32'd0;
				tx_gain <= 6'd0;
				run_radio <= 1'b0;
			end
	endcase
end

logic radioberry_ready;
assign radioberry_ready = ~(network_state_dhcp & network_state_fixedip);

spi_slave spi_slave_rx_inst(.rstb(!reset),.ten(1'b1),.tdata({radioberry_ready, run_radio, 14'b0, radioberry_ip}),.mlb(1'b1),.ss(spi_ce[0]),.sck(spi_sck),.sdin(spi_mosi), .sdout(spi_miso),.done(spi_done),.rdata(spi0_recv));


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

logic [1:0] rx1_speed;
logic [1:0] rx2_speed;	
logic [5:0] rx1_rate;

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
//                          rxFIFO Handler (IQ Samples) rx
//------------------------------------------------------------------------------------------------------------------------------------------------------------		
logic [10:0] fifo_rx_length;	
logic [47:0] fifo_rx_data;
logic rx_request;

rxFIFO rx_FIFO_inst(	.aclr(reset),
							.wrclk(clk_ad9866),.data({rx_I, rx_Q}),.wrreq(rx_strobe), .wrusedw(fifo_rx_length), .wrfull(),  
							.rdclk(phy_clk_div4),.q(fifo_rx_data),.rdreq(rx_request));	
		
wire clk_envelope;	
wire clk_internal;															
ad9866pll ad9866pll_inst (.inclk0(clk_76m8), .c0(clk_ad9866), .c1(clk_ad9866_2x), .c2(clk_envelope), .c3(clk_internal),  .locked());
								
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          Running...
//------------------------------------------------------------------------------------------------------------------------------------------------------------	
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