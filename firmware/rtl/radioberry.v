// Project			: Radioberry
//
// Module			: Top level design radioberry.v
//
// Target Devices	: Cyclone III
//
// Tool 		 		: Quartus V12.1 Free WebEdition
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description: 
//
//				Radioberry SDR firmware code.
//
//
// Johan Maas PA3GSB 
//
// Date:    27 December 2015
//				First version.
//------------------------------------------------------------------------------------------------------------------------------------------------------------

`include "timescale.v"

module radioberry(
clk_10mhz, 
ad9866_clk, ad9866_adio,ad9866_rxen,ad9866_rxclk,ad9866_txen,ad9866_txclk,ad9866_sclk,ad9866_sdio,ad9866_sdo,ad9866_sen_n,ad9866_rst_n,ad9866_mode,	
spi_sck, spi_mosi, spi_miso, spi_ce,   
DEBUG_LED1,DEBUG_LED2,DEBUG_LED3,DEBUG_LED4,
rx1_FIFOEmpty, rx2_FIFOEmpty,
txFIFOFull,
ptt_in,
ptt_out,
filter);

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
output wire rx1_FIFOEmpty;
output wire rx2_FIFOEmpty;
output wire txFIFOFull;

output  wire  DEBUG_LED1;  
output  wire  DEBUG_LED2;  
output  wire  DEBUG_LED3;  
output  wire  DEBUG_LED4;  // TX indicator...

input wire ptt_in;
output wire ptt_out;
output [6:0] filter; 

//ATT
reg   [4:0] att;           // 0-31 dB attenuator value
reg 	dither;					// if 0 than 32db additional gain.
reg 	randomize;				// if randomize is checked (eg in powersdr) the agc value is used for gain
									// if randomize is not checked (eg in powersdr) the gain value (inversie van s-att) is used for gain

//Debug	
assign DEBUG_LED3 =  (tx_freq == 32'd3630000) ? 1'b1:1'b0; 


//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         AD9866 Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------

assign ad9866_mode = 1'b0;				//HALFDUPLEX
assign ad9866_rst_n = ~reset;
assign ad9866_adio = ptt_in ? DAC[13:2] : 12'bZ;
assign ad9866_rxclk = ad9866_clk;	 
assign ad9866_txclk = ad9866_clk;	 

assign ad9866_rxen = (~ptt_in) ? 1'b1: 1'b0;
assign ad9866_txen = (ptt_in) ? 1'b1: 1'b0;

assign ptt_out = ptt_in;

wire ad9866rqst;
reg [5:0] tx_gain;

reg [5:0] prev_gain;
always @ (posedge clk_10mhz)
    prev_gain <= tx_gain;

assign ad9866rqst = tx_gain != prev_gain;

ad9866 ad9866_inst(.reset(reset),.clk(clk_10mhz),.sclk(ad9866_sclk),.sdio(ad9866_sdio),.sdo(ad9866_sdo),.sen_n(ad9866_sen_n),.dataout(),.extrqst(ad9866rqst),.gain(tx_gain));

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         SPI Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------

wire [47:0] spi_recv;
wire spi_done;

always @ (posedge spi_done)
begin	
	if (!ptt_in) begin
		rx1_freq <= spi_recv[31:0];
		rx1_speed <= spi_recv[41:40];
		
		//following settings are from the active selected receiver! transferred with the rx1 data
		att <= spi_recv[36:32];
		dither <= spi_recv[37];
		randomize <= spi_recv[38];
		
	end else begin
		tx_gain <= spi_recv[37:32];
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
		rx2_freq <= spi_rx2_recv[31:0];
		rx2_speed <= spi_rx2_recv[41:40];
	end else begin
		tx_freq <= spi_rx2_recv[31:0];
	end	
end 

spi_slave spi_slave_rx2_inst(.rstb(!reset),.ten(1'b1),.tdata(rx2_DataFromFIFO),.mlb(1'b1),.ss(spi_ce[1]),.sck(spi_sck),.sdin(spi_mosi), .sdout(spi_miso),.done(spi_rx2_done),.rdata(spi_rx2_recv));

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         Decimation Rate Control common
//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Decimation rates
localparam RATE48 = 6'd24;
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
 
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         GAIN Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire rxclipp = (ad9866_adio == 12'b011111111111);
wire rxclipn = (ad9866_adio == 12'b100000000000);
wire rxnearclip = (ad9866_adio[11:8] == 4'b0111) | (ad9866_adio[11:8] == 4'b1000);
wire rxgoodlvlp = (ad9866_adio[11:9] == 3'b011);
wire rxgoodlvln = (ad9866_adio[11:9] == 3'b100);

reg agc_nearclip;
reg agc_goodlvl;
reg [25:0] agc_delaycnt;
reg [5:0] agc_value;
wire agc_clrnearclip;
wire agc_clrgoodlvl;

always @(posedge ad9866_clk)
begin
    if (agc_clrnearclip) agc_nearclip <= 1'b0;
    else if (rxnearclip) agc_nearclip <= 1'b1;
end

always @(posedge ad9866_clk)
begin
    if (agc_clrgoodlvl) agc_goodlvl <= 1'b0;
    else if (rxgoodlvlp | rxgoodlvln) agc_goodlvl <= 1'b1;
end

always @(posedge ad9866_clk)
begin
    agc_delaycnt <= agc_delaycnt + 1;
end

always @(posedge ad9866_clk)
begin
    if (reset) 
        agc_value <= 6'b011111;
    // Decrease gain if near clip seen
    else if ( ((agc_clrnearclip & agc_nearclip & (agc_value != 6'b000000)) | agc_value > gain_value ) & ~ptt_in ) 
        agc_value <= agc_value - 6'h01;
    // Increase if not in the sweet spot of seeing agc_nearclip
    // But no more than ~26dB (38) as that is the place of diminishing returns re the datasheet
    else if ( agc_clrgoodlvl & ~agc_goodlvl & (agc_value <= gain_value) & ~ptt_in )
        agc_value <= agc_value + 6'h01;
end

// tp = 1.0/61.44e6
// 2**26 * tp = 1.0922 seconds
// PGA settling time is less than 500 ns
// Do decrease possible every 2 us (2**7 * tp)
assign agc_clrnearclip = (agc_delaycnt[6:0] == 7'b1111111);
// Do increase possible every 68 ms, 1us before/after a possible descrease
assign agc_clrgoodlvl = (agc_delaycnt[21:0] == 22'b1011111111111110111111);


wire [5:0] gain_value;
assign gain_value = {~dither, ~att};

//assign ad9866_pga = randomize ? agc_value : gain_value;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         FILTER Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------

filter filter_inst(.clock(clk_10mhz), .frequency(sync_phase_word), .selected_filter(filter));

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         Convert frequency to phase word 
//
//		Calculates  ratio = fo/fs = frequency/73.728Mhz where frequency is in MHz
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire   [31:0] sync_phase_word;
wire  [63:0] ratio;

reg[31:0] rx1_freq;

localparam M2 = 32'd1954687338; 	// B57 = 2^57.   M2 = B57/CLK_FREQ = 73728000
localparam M3 = 32'd16777216;   	// M3 = 2^24, used to round the result
assign ratio = rx1_freq * M2 + M3; 
assign sync_phase_word = ratio[56:25]; 

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         Convert frequency to phase word rx2
//
//		Calculates  ratio = fo/fs = frequency/73.728Mhz where frequency is in MHz
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire   [31:0] sync_phase_word_rx2;
wire  [63:0] ratio_rx2;

reg[31:0] rx2_freq;

localparam M4 = 32'd1954687338; 	// B57 = 2^57.   M2 = B57/CLK_FREQ = 73728000
localparam M5 = 32'd16777216;   	// M3 = 2^24, used to round the result

assign ratio_rx2 = rx2_freq * M4 + M5; 
assign sync_phase_word_rx2 = ratio_rx2[56:25]; 

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         Convert frequency to phase word tx
//
//		Calculates  ratio = fo/fs = frequency/73.728Mhz where frequency is in MHz
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire   [31:0] sync_phase_word_tx;
wire  [63:0] ratio_tx;

reg[31:0] tx_freq;

localparam M6 = 32'd1954687338; 	// B57 = 2^57.   M2 = B57/CLK_FREQ = 73728000
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

localparam CICRATE = 6'd08;

receiver #(.CICRATE(CICRATE)) 
		receiver_inst(	.clock(ad9866_clk),
						.rate(rx1_rate), 
						.frequency(sync_phase_word),
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

localparam CICRATE_RX2 = 6'd08;

receiver #(.CICRATE(CICRATE_RX2)) 
		receiver_rx2_inst(	
						.clock(ad9866_clk),
						.rate(rx2_rate), 
						.frequency(sync_phase_word_rx2),
						.out_strobe(rx2_strobe),
						.in_data(adcpipe[1]),
						.out_data_I(rx2_I),
						.out_data_Q(rx2_Q));			

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          rxFIFO Handler (IQ Samples) rx1
//------------------------------------------------------------------------------------------------------------------------------------------------------------
reg [47:0] rxDataFromFIFO;

wire rx1req = ptt_in ? 1'b0 : 1'b1;

rxFIFO rxFIFO_inst(	.aclr(reset),
							.wrclk(ad9866_clk),.data({rx_I, rx_Q}),.wrreq(rx_strobe), .wrempty(rx1_FIFOEmpty), 
							.rdclk(~spi_ce[0]),.q(rxDataFromFIFO),.rdreq(rx1req));



//always @(posedge ad9866_clk)
//begin	
//	if (rx_strobe) begin
//		rxDataFromFIFO <= {rx_I, rx_Q};
//		rx1_FIFOEmpty <= 0;
//	end else begin
//		if (~spi_ce[0]) 
//			rx1_FIFOEmpty <= 1;
//	
//	end
//end


	
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          rxFIFO Handler (IQ Samples) rx2
//------------------------------------------------------------------------------------------------------------------------------------------------------------
reg [47:0] rx2_DataFromFIFO;

wire rx2req = ptt_in ? 1'b0 : 1'b1;

rxFIFO rx2_FIFO_inst(.aclr(reset),
							.wrclk(ad9866_clk),.data({rx2_I, rx2_Q}),.wrreq(rx2_strobe), .wrempty(rx2_FIFOEmpty), 
							.rdclk(~spi_ce[1]),.q(rx2_DataFromFIFO),.rdreq(rx2req));	

//always @(posedge ad9866_clk)
//begin	
//	if (rx2_strobe) begin
//		rx2_DataFromFIFO <= {rx2_I, rx2_Q};
//		rx2_FIFOEmpty <= 0;
//	end else begin
//		if (~spi_ce[0]) 
//			rx2_FIFOEmpty <= 1;
//	
//	end
//end

							
				
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          txFIFO Handler ( IQ-Transmit)
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire wtxreq = ptt_in ? 1'b1 : 1'b0;

txFIFO txFIFO_inst(	.aclr(reset), 
							.wrclk(~spi_ce[0]), .data(spi_recv[31:0]), .wrreq(wtxreq),
							.rdclk(ad9866_clk), .q(txDataFromFIFO), .rdreq(txFIFOReadStrobe),  .rdempty(txFIFOEmpty), .rdfull(txFIFOFull));
	

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                        Transmitter module
//------------------------------------------------------------------------------------------------------------------------------------------------------------							
wire [31:0] txDataFromFIFO;
wire txFIFOEmpty;
wire txFIFOReadStrobe;

transmitter transmitter_inst(.reset(reset), .clk(ad9866_clk), .frequency(sync_phase_word_tx), 
							 .afTxFIFO(txDataFromFIFO), .afTxFIFOEmpty(txFIFOEmpty), .afTxFIFOReadStrobe(txFIFOReadStrobe),
							.out_data(DAC), .PTT(ptt_in), .LED(DEBUG_LED4));	

wire [13:0] DAC;
	
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

assign {DEBUG_LED1,DEBUG_LED2} = counter[23:22];

endmodule