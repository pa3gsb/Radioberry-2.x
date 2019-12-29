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
//				Radioberry v2.0 SDR firmware code for the TINKER BOARD.
//
//	
//	Tinkerboard is a SBC (single board computer) produced by ASUS.
//
// 	https://www.asus.com/us/Single-Board-Computer/Tinker-Board/
// 	
//	
//	This firmware puts the radioberry into full duplex. (you need the beta 3 including the small modification).
//
//  Using SPI bus (no bitbanging), no seperate version needed anymore for the standalone and emulator setup! All combined in one!
//
//	The radioberry firmware is formed by the gateware radioberry.rbf (verilog) and the tinkerware tinkerradio (c-language) executable.
//	Radioberry.rbf firmware must be loaded into the FPGA by using the loadFPGA utility. 
// 	Tinkerradio must be started in the linux tinkerboard user space. 
// 	
//	Johan Maas PA3GSB 
//
//	Date:   	26 June 2019
//	
//------------------------------------------------------------------------------------------------------------------------------------------------------------

`include "timescale.v"

module radioberry(
clk_76m8,
ad9866_clk,ad9866_rx,ad9866_tx,ad9866_rxsync,ad9866_rxclk,ad9866_txsync,ad9866_txquietn,ad9866_sclk,ad9866_sdio,ad9866_sdo,ad9866_sen_n,ad9866_rst_n,ad9866_mode,	
spi_sck, spi_mosi, spi_miso, spi_ce,  
rx1_samples,
ptt_in, ptt_out);

input wire clk_76m8;			
input wire ad9866_clk;

input [5:0] ad9866_rx;
output [5:0] ad9866_tx;
input ad9866_rxsync;
input ad9866_rxclk;
output ad9866_txsync;
output ad9866_txquietn;

output wire ad9866_sclk;
output wire ad9866_sdio;
input  wire ad9866_sdo;
output wire ad9866_sen_n;
output wire ad9866_rst_n;
output ad9866_mode;

// SPI connect to Raspberry PI SPI-0.
input wire spi_sck;
input wire spi_mosi; 
output wire spi_miso; 
input [2:0] spi_ce; 

//rx data available
output wire rx1_samples;

input wire ptt_in;
output wire ptt_out;

logic clk_ad9866;
logic clk_ad9866_2x;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         Radioberry Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------
reg[31:0] rx1_phase_word;
reg[31:0] rx2_phase_word;
reg[31:0] tx_phase_word;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         AD9866 Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------
assign ad9866_mode = 1'b1;				//FULLDUPLEX
assign ad9866_rst_n = ~reset;
assign ptt_out = ptt_in;
	 
// RX Path
logic	[11:0]	rx_data_assemble;
logic	[5:0]	rffe_ad9866_rx_d1, rffe_ad9866_rx_d2;
logic			rffe_ad9866_rxsync_d1;

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

assign ptt_out = ptt_in;

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
//                         SPI RX1
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire [47:0] spi_rx1_recv;
wire spi_rx1_done;

always @ (posedge spi_rx1_done)
begin	
	rx1_phase_word <= spi_rx1_recv[31:0];
	rx1_speed <= spi_rx1_recv[41:40];
	rx_gain <= ~spi_rx1_recv[37:32];
end 

spi_slave spi_slave_rx1_inst(.rstb(!reset),.ten(1'b1),.tdata(rx1DataFromFIFO),.mlb(1'b1),.ss(spi_ce[0]),.sck(spi_sck),.sdin(spi_mosi), .sdout(spi_miso),.done(spi_rx1_done),.rdata(spi_rx1_recv));

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         SPI TX 
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire [31:0] spi_tx_recv;
reg [31:0] tx_iq;
wire spi_tx_done;

always @ (posedge spi_tx_done) tx_iq <= spi_tx_recv[31:0];
		
spi_slave #(.WIDTH(32)) spi_slave_tx_inst(.rstb(!reset),.ten(1'b1),.tdata({tx_rd_length}),.mlb(1'b1),.ss(spi_ce[1]),.sck(spi_sck),.sdin(spi_mosi), .sdout(spi_miso),.done(spi_tx_done),.rdata(spi_tx_recv));

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         SPI Control tx
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire [47:0] spi_tx_control_recv;
wire spi_tx_control_done;

always @ (posedge spi_tx_control_done) 
begin
	tx_phase_word <= spi_tx_control_recv[31:0];
	tx_gain <= ~spi_tx_control_recv[37:32];
end
		
spi_slave spi_slave_tx_control_inst(.rstb(!reset),.ten(1'b1),.tdata(47'b0),.mlb(1'b1),.ss(spi_ce[2]),.sck(spi_sck),.sdin(spi_mosi), .sdout(spi_miso),.done(spi_tx_control_done),.rdata(spi_tx_control_recv));


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
wire	[23:0] rx1_I;
wire	[23:0] rx1_Q;
wire	rx1_strobe;

localparam CICRATE = 6'd05;

receiver #(.CICRATE(CICRATE)) 
		receiver_rx_inst(	
						.clock(clk_ad9866),
						.clock_2x(clk_ad9866_2x),
						.rate(rx1_rate), 
						.frequency(rx1_phase_word),
						.out_strobe(rx1_strobe),
						.in_data(adcpipe[0]),
						.out_data_I(rx1_I),
						.out_data_Q(rx1_Q));

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          rx1FIFO Handler (IQ Samples) rx1
//------------------------------------------------------------------------------------------------------------------------------------------------------------
reg [47:0] rx1DataFromFIFO;
wire [10:0] rx1_wr_length;
assign rx1_samples = (rx1_wr_length > 11'd63) ? 1'b1: 1'b0; 

rxFIFO rx1_FIFO_inst(	.aclr(reset),
							.wrclk(clk_ad9866),.data({rx1_I, rx1_Q}),.wrreq(rx1_strobe), .wrusedw(rx1_wr_length), .wrfull(),  
							.rdclk(spi_rx1_done),.q(rx1DataFromFIFO ),.rdreq(1'b1));	

								
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


//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                        Radioberry internal frequency module
//------------------------------------------------------------------------------------------------------------------------------------------------------------	
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