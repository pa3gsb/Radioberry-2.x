// Project			: Radioberry
//
// Module			: Top level design radioberry.v
//
// Target Devices	: Cyclone 10LP
//
// Tool 		 		: Quartus Prime Lite Edition v17.0.2
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description: 
//
//				Radioberry v2.0 SDR firmware code.
//
// Johan Maas PA3GSB 
//
// Date:    20 Februari 2018
//	
//------------------------------------------------------------------------------------------------------------------------------------------------------------

`include "timescale.v"

module radioberry(
clk_10mhz, 
ad9866_clk, ad9866_adio,ad9866_rxen,ad9866_rxclk,ad9866_txen,ad9866_txclk,ad9866_sclk,ad9866_sdio,ad9866_sdo,ad9866_sen_n,ad9866_rst_n,ad9866_mode,	
spi_sck, spi_mosi, spi_miso, spi_ce,   
rb_info_1,rb_info_2,
rx1_samples, 
txFIFOFull,
ptt_in,
ptt_out,
filter,
KEY_DOT, KEY_DASH, key_dot_rpi, key_dash_rpi, cw_ptt);

input wire clk_10mhz;			
input wire ad9866_clk;
inout [11:0] ad9866_adio;
output wire ad9866_rxen;
output wire ad9866_rxclk;
output wire ad9866_txen;
output wire ad9866_txclk;
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
input [1:0] spi_ce; 
output wire rx1_samples;
output wire txFIFOFull;

output  wire  rb_info_1;  // radioberry info-1;  checks 10 Mhz clock 
output  wire  rb_info_2;  // radioberry info-2;  checks ad9866 clock (in tx flashes 2 times faster)
 
input wire ptt_in;
output wire ptt_out;
output [6:0] filter; 

input  wire KEY_DOT;  		//dot input from external input
input  wire KEY_DASH;      //dash input from external input
output wire key_dot_rpi;
output wire key_dash_rpi;
output wire cw_ptt;
reg cw_fpga;
wire icw_ptt;
assign cw_ptt = (icw_ptt && cw_fpga) ? 1'b1: 1'b0; //cw by fpga is set inform emulator

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         AD9866 Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------

assign ad9866_mode = 1'b0;				//HALFDUPLEX
assign ad9866_rst_n = ~reset;
assign ad9866_adio = ptt_in ? DACDp : 12'bZ;
assign ad9866_rxclk = ad9866_clk;	 
assign ad9866_txclk = ad9866_clk;	 

assign ad9866_rxen = (~ptt_in) ? 1'b1: 1'b0;
assign ad9866_txen = (ptt_in) ? 1'b1: 1'b0;

assign ptt_out = ptt_in;

wire ad9866_rx_rqst;
wire ad9866_tx_rqst;
reg [5:0] rx_gain;
reg [5:0] tx_gain;

assign ad9866_rx_rqst = (!ptt_in && gain_update && ad9866_sen_n);
assign ad9866_tx_rqst = (ptt_in && gain_update && ad9866_sen_n);

ad9866 ad9866_inst(.reset(reset),.clk(clk_10mhz),.sclk(ad9866_sclk),.sdio(ad9866_sdio),.sdo(ad9866_sdo),.sen_n(ad9866_sen_n),.dataout(),.ext_tx_rqst(ad9866_tx_rqst),.tx_gain(tx_gain),.ext_rx_rqst(ad9866_rx_rqst),.rx_gain(rx_gain));


//--------------------------------------------------------------------------------------------
//  	Iambic CW Keyer
//--------------------------------------------------------------------------------------------
// passing the keying actions (dot and dash) to the RPI for setting the sidetone.
assign key_dot_rpi = KEY_DOT;
assign key_dash_rpi = KEY_DASH;

wire clk_192K;
wire clk_30K;
PLL_IAMBIC_10 PLL_IAMBIC_inst (.inclk0(clk_10mhz), .c0(clk_192K), .c1(clk_30K), .c2(),  .c3(), .locked());

reg [5:0] 	cw_speed; 			// CW keyer speed 0-60 WPM
reg [1:0] 	iambic_mode;		// 00 = straight/bug, 01 = Mode A, 10 = Mode B
reg [6:0] 	keyer_weight;		// keyer weight 33-66
reg			keyer_revers;		// reverse keyer
wire			keyout;

iambic #(30) iambic_inst (	.clock(clk_30K), .cw_speed(cw_speed),  .iambic_mode(iambic_mode), .weight({1'b0, keyer_weight}), 
							.letter_space(1'b0), .dot_key(!KEY_DOT), .dash_key(!KEY_DASH),
							.CWX(1'b0), .paddle_swap(keyer_revers), .keyer_out(keyout));
								  							  
//--------------------------------------------------------------------------------------------
//  	Calculate  Raised Cosine profile for sidetone and CW envelope when internal CW selected 
//--------------------------------------------------------------------------------------------
wire [15:0] CW_RF;
//wire   [7:0] delay;				// 0 - 255, sets delay in mS from CW Key activation to RF out
//wire   [9:0] hang;				// 0 - 1000, sets delay in mS from release of CW Key to dropping of PTT

profile profile_CW(.clock(clk_192K), .CW_char(keyout), .profile(CW_RF), .delay(8'd20), .hang(10'd300), .PTT(icw_ptt));	
		

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         SPI Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------

wire [47:0] spi_recv;
wire spi_done;
reg [31:0] tx_iq;

always @ (posedge spi_done)
begin	
	if (!ptt_in) begin
		rx1_freq <= spi_recv[31:0];
		rx1_speed <= spi_recv[41:40];
		rx_gain <= ~spi_recv[37:32];
		cw_fpga <= spi_recv[38];
	end else begin
		tx_gain <= spi_recv[37:32];
		tx_iq <= spi_recv[31:0];
	end
end 

spi_slave spi_slave_rx_inst(.rstb(!reset),.ten(1'b1),.tdata(rxDataFromFIFO),.mlb(1'b1),.ss(spi_ce[0]),.sck(spi_sck),.sdin(spi_mosi), .sdout(spi_miso),.done(spi_done),.rdata(spi_recv));


//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         SPI Control rx2
//------------------------------------------------------------------------------------------------------------------------------------------------------------

wire [47:0] spi_rx2_recv;
wire spi_rx2_done;

always @ (posedge spi_rx2_done)
begin	
	if (!ptt_in) begin
		//rx2_freq <= spi_rx2_recv[31:0];
		//rx2_speed <= spi_rx2_recv[41:40];
	end else begin
		iambic_mode <= spi_rx2_recv[47:46];
		cw_speed <= spi_rx2_recv[45:40];
		keyer_revers <= spi_rx2_recv[39:39];
		keyer_weight <= spi_rx2_recv[38:32];
		tx_freq <= spi_rx2_recv[31:0];
	end	
end 

spi_slave spi_slave_rx2_inst(.rstb(!reset),.ten(1'b1),.tdata(rx2_DataFromFIFO),.mlb(1'b1),.ss(spi_ce[1]),.sck(spi_sck),.sdin(spi_mosi), .sdout(spi_miso),.done(spi_rx2_done),.rdata(spi_rx2_recv));

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
//                         FILTER Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------

filter filter_inst(.clock(clk_10mhz), .frequency(rx1_freq), .selected_filter(filter));

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         Convert frequency to phase word 
//
//		Calculates  ratio = fo/fs = frequency/76.8Mhz where frequency is in MHz
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire   [31:0] sync_phase_word;
wire  [63:0] ratio;

reg[31:0] rx1_freq;
					    
localparam M2 = 32'd1876499845; 	// B57 = 2^57.   M2 = B57/CLK_FREQ = 76800000
localparam M3 = 32'd16777216;   	// M3 = 2^24, used to round the result
assign ratio = rx1_freq * M2 + M3; 
assign sync_phase_word = ratio[56:25]; 

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         Convert frequency to phase word tx
//
//		Calculates  ratio = fo/fs = frequency/73.728Mhz where frequency is in MHz
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire   [31:0] sync_phase_word_tx;
wire  [63:0] ratio_tx;

reg[31:0] tx_freq;

localparam M6 = 32'd1876499845; 	// B57 = 2^57.   M2 = B57/CLK_FREQ = 76800000
localparam M7 = 32'd16777216;   	// M3 = 2^24, used to round the result

assign ratio_tx = tx_freq * M6 + M7; 
assign sync_phase_word_tx = ratio_tx[56:25]; 

//------------------------------------------------------------------------------
//                           Software Reset Handler
//------------------------------------------------------------------------------
wire reset;
reset_handler reset_handler_inst(.clock(clk_10mhz), .reset(reset));

//------------------------------------------------------------------------------
//                           Pipeline for adc fanout
//------------------------------------------------------------------------------
reg [11:0] adcpipe [0:1];

always @ (posedge ad9866_clk) begin
    adcpipe[0] <= ad9866_adio;
    adcpipe[1] <= ad9866_adio;
end

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                        Receiver module rx1 
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire	[23:0] rx_I;
wire	[23:0] rx_Q;
wire	rx_strobe;

localparam CICRATE = 6'd05;

receiver #(.CICRATE(CICRATE)) 
		receiver_rx_inst(	.clock(ad9866_clk),
						.rate(rx1_rate), 
						.frequency(sync_phase_word),
						.out_strobe(rx_strobe),
						.in_data(adcpipe[0]),
						.out_data_I(rx_I),
						.out_data_Q(rx_Q));
						
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          rxFIFO Handler (IQ Samples) rx
//------------------------------------------------------------------------------------------------------------------------------------------------------------
reg [47:0] rxDataFromFIFO;

wire rxreq = ptt_in ? 1'b0 : 1'b1;
wire [10:0] rx_wr_length;
assign rx1_samples = (rx_wr_length > 11'd63) ? 1'b1: 1'b0; 

wire rx_wrfull;

rxFIFO rx_FIFO_inst(	.aclr(reset),
							.wrclk(ad9866_clk),.data({rx_I, rx_Q}),.wrreq(rx_strobe), .wrusedw(rx_wr_length), .wrfull(rx_wrfull),  
							.rdclk(spi_done),.q(rxDataFromFIFO),.rdreq(rxreq));					
		
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          txFIFO Handler ( IQ-Transmit)
//------------------------------------------------------------------------------------------------------------------------------------------------------------
reg [11:0]  tx_rd_length;
wire empty;

assign txFIFOFull = (tx_rd_length > 12'd4094) ? 1'b1: 1'b0;

wire txreq = ptt_in ? 1'b1 : 1'b0 ;

txFIFO txFIFO_inst(	.aclr(reset), 
							.wrclk(~spi_ce[0]), .data(tx_iq), .wrreq(txreq), .wrfull (), .wrusedw (), 
							.rdclk(ad9866_clk), .q(txDataFromFIFO), .rdreq((txFIFOReadStrobe & ~empty)),  .rdempty(empty), .rdfull() , .rdusedw(tx_rd_length));
	
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                        Transmitter module
//------------------------------------------------------------------------------------------------------------------------------------------------------------							
wire [31:0] txDataFromFIFO;
wire txFIFOReadStrobe;
wire [31:0] txData;

transmitter transmitter_inst(	.reset(reset), .clk(ad9866_clk), .frequency(sync_phase_word_tx), 
								.tsiq_data(txDataFromFIFO), .tsiq_read_strobe(txFIFOReadStrobe), .tsiq_valid(~empty), .CW_RF(CW_RF), 
								.out_data(DAC), .PTT(ptt_in), .CW_PTT(cw_ptt), .LED(rb_info_2));

wire [11:0] DAC;

reg [11:0] DACDp;
always @ (negedge ad9866_clk) DACDp <= DAC;
	
//------------------------------------------------------------------------------
//                          Running...
//------------------------------------------------------------------------------
reg [26:0]counter;


always @(posedge clk_10mhz) 
begin
  if (reset)
    counter <= 26'b0;
  else
    counter <= counter + 1'b1;
end

assign rb_info_1 = counter[23];
wire gain_update;
assign gain_update = counter[22];

endmodule