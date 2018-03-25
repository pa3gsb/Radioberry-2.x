// Project			: Radioberry
//
// Module			: Top level design radioberry.v
//
// Target Devices	: Cyclone 10LP
//
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description: 
//
//				Radioberry WSPR-SDR firmware code.
//		
//				The firmware suppports 3 wspr rx channels (cl025 as target)
//				and 2 wspr rx channels (cl016 as target).
//
//				Johan Maas PA3GSB
//------------------------------------------------------------------------------------------------------------------------------------------------------------

`include "timescale.v"

module radioberry(
clk_10mhz, 
ad9866_clk, ad9866_adio,ad9866_rxen,ad9866_rxclk,ad9866_txen,ad9866_txclk,ad9866_sclk,ad9866_sdio,ad9866_sdo,ad9866_sen_n,ad9866_rst_n,ad9866_mode,	
spi_sck, spi_mosi, spi_miso, spi_ce,   
rb_info_1,rb_info_2,
rx_FIFOEmpty);

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
output wire rx_FIFOEmpty;

output  wire  rb_info_1;  
output  wire  rb_info_2;  
 		
			
//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         AD9866 Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------

assign ad9866_mode = 1'b0;				//HALFDUPLEX
assign ad9866_rst_n = ~reset;
assign ad9866_adio = 12'bZ;
assign ad9866_rxclk = ad9866_clk;	 
assign ad9866_txclk = ad9866_clk;	 

assign ad9866_rxen = 1'b1;
assign ad9866_txen = 1'b0;

wire ad9866_rx_rqst;
wire ad9866_tx_rqst;
reg [5:0] rx_gain;
reg [5:0] tx_gain;

reg [5:0] prev_rx_gain;
reg [5:0] prev_tx_gain;
always @ (posedge clk_10mhz)
begin
	prev_rx_gain <= rx_gain;
	prev_tx_gain <= tx_gain;
end

assign ad9866_rx_rqst = rx_gain != prev_rx_gain;
assign ad9866_tx_rqst = tx_gain != prev_tx_gain;

ad9866 ad9866_inst(.reset(reset),.clk(clk_10mhz),.sclk(ad9866_sclk),.sdio(ad9866_sdio),.sdo(ad9866_sdo),.sen_n(ad9866_sen_n),.dataout(),.ext_tx_rqst(ad9866_tx_rqst),.tx_gain(tx_gain),.ext_rx_rqst(ad9866_rx_rqst),.rx_gain(rx_gain));


//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         SPI Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire [47:0] spi_recv;
wire spi_done;
reg [1:0] nnrx;

always @ (posedge spi_done)
begin	
	rx_gain <= ~spi_recv[37:32];
	nnrx <= spi_recv[43:42];
end 

always @ (negedge spi_done)
begin
	rx_freq[nnrx] <= spi_recv[31:0];
end

spi_slave spi_slave_rx_inst(.rstb(!reset),.ten(1'b1),.tdata(rxDataFromFIFO[nnrx]),.mlb(1'b1),.ss(spi_ce[0]),.sck(spi_sck),.sdin(spi_mosi), .sdout(spi_miso),.done(spi_done),.rdata(spi_recv));


//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         Convert frequency to phase word 
//
//		Calculates  ratio = fo/fs = frequency/73.728Mhz where frequency is in MHz
//
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire   [31:0] sync_phase_word [0:NR-1];
wire  [63:0] ratio [0:NR-1];

reg[31:0] rx_freq[0:NR-1];

		
localparam M2 = 32'd1876499845; 	// B57 = 2^57.   M2 = B57/CLK_FREQ = 76800000
localparam M3 = 32'd16777216;   	// M3 = 2^24, used to round the result


generate
  genvar n;
  for (n = 0; n < NR; n = n + 1) // calc freq phase word for nRx Channel
   begin: MDC
		assign ratio[n] = rx_freq[n] * M2 + M3; 
		assign sync_phase_word[n] = ratio[n][56:25]; 
  end
endgenerate

//------------------------------------------------------------------------------
//                           Software Reset Handler
//------------------------------------------------------------------------------
wire reset;
reset_handler reset_handler_inst(.clock(clk_10mhz), .reset(reset));

//------------------------------------------------------------------------------
//                           Pipeline for adc fanout
//------------------------------------------------------------------------------
reg [11:0] adcpipe [0:3];
always @ (posedge ad9866_clk) begin
	adcpipe[0] <= ad9866_adio;
	adcpipe[1] <= ad9866_adio;
	adcpipe[2] <= ad9866_adio;
	adcpipe[3] <= ad9866_adio;
end

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                        Receiver module nRx
//------------------------------------------------------------------------------------------------------------------------------------------------------------
localparam NR = 3; // Number of Receivers 

wire	[23:0] rx_I [0:NR-1];
wire	[23:0] rx_Q [0:NR-1];
wire	rx_strobe [0:NR-1];

localparam CICRATE = 6'd16;

wire [47:0] rxDataFromFIFO [0:NR-1];
wire emptyFIFO[0:NR-1];

generate
  genvar c;
  for (c = 0; c < NR; c = c + 1) 
   begin: NRX
		receiver #(.CICRATE(CICRATE)) receiver_inst(	.clock(ad9866_clk),
														.rate(6'd24), 
														.frequency(sync_phase_word[c]),
														.out_strobe(rx_strobe[c]),
														.in_data(adcpipe[c]),
														.out_data_I(rx_I[c]),
														.out_data_Q(rx_Q[c]));
														
		rxFIFO rxFIFO_inst(	.aclr(reset),
							.wrclk(ad9866_clk),.data({rx_I[c], rx_Q[c]}),.wrreq(rx_strobe[c]), .wrempty(emptyFIFO[c]), 
							.rdclk(~spi_ce[0]),.q(rxDataFromFIFO[c]),.rdreq(nnrx==c ? 1'b1 : 1'b0 ));
														
  end
endgenerate
	
assign rx_FIFOEmpty = emptyFIFO[0] || emptyFIFO[1] || emptyFIFO[2];						
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

assign {rb_info_1,rb_info_2} = counter[23:22];

endmodule