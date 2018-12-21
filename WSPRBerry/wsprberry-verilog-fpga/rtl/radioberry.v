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
//
// Johan Maas PA3GSB 
//
// Date:    21 December 2018
//	
//------------------------------------------------------------------------------------------------------------------------------------------------------------

`include "timescale.v"

module radioberry(
clk_76m8,
ad9866_clk,ad9866_rx,ad9866_tx,ad9866_rxsync,ad9866_rxclk,ad9866_txsync,ad9866_txquietn,ad9866_sclk,ad9866_sdio,ad9866_sdo,ad9866_sen_n,ad9866_rst_n,ad9866_mode,	
spi_sck, spi_mosi, spi_miso, spi_ce,   
rx_FIFOEmpty);

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
input [1:0] spi_ce; 


//rx data
output wire rx_FIFOEmpty; 

logic clk_ad9866;
logic clk_ad9866_2x;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         AD9866 Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------

assign ad9866_mode = 1'b1;				//FULLDUPLEX
assign ad9866_rst_n = ~reset;
	 
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
  tx_en_d1 <= 1'b0;
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


reg [5:0] prev_rx_gain;
reg [5:0] prev_tx_gain;
always @ (posedge clk_internal)
begin
	prev_rx_gain <= rx_gain;
	prev_tx_gain <= tx_gain;
end

assign ad9866_rx_rqst = rx_gain != prev_rx_gain;
assign ad9866_tx_rqst = tx_gain != prev_tx_gain;

ad9866 ad9866_inst(.reset(reset),.clk(clk_internal),.sclk(ad9866_sclk),.sdio(ad9866_sdio),.sdo(ad9866_sdo),.sen_n(ad9866_sen_n),.dataout(),.ext_tx_rqst(ad9866_tx_rqst),.tx_gain(tx_gain),.ext_rx_rqst(ad9866_rx_rqst),.rx_gain(rx_gain));


//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                         SPI Control
//------------------------------------------------------------------------------------------------------------------------------------------------------------
wire [47:0] spi_recv;
wire spi_done;
reg [2:0] nnrx;

always @ (posedge spi_done)
begin	
	rx_gain <= ~spi_recv[37:32];
	nnrx <= spi_recv[44:42];
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
reset_handler reset_handler_inst(.clock(clk_internal), .reset(reset));

//------------------------------------------------------------------------------
//                           Pipeline for adc fanout
//------------------------------------------------------------------------------
reg [11:0] adcpipe [0:7];

always @ (posedge clk_ad9866) begin
	adcpipe[0] <= rx_data_assemble;
   adcpipe[1] <= rx_data_assemble;
	adcpipe[2] <= rx_data_assemble;
	adcpipe[3] <= rx_data_assemble;
   adcpipe[4] <= rx_data_assemble;
	adcpipe[5] <= rx_data_assemble;
	adcpipe[6] <= rx_data_assemble;
	adcpipe[7] <= rx_data_assemble;
end

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                        Receiver module nRx
//------------------------------------------------------------------------------------------------------------------------------------------------------------
`ifdef wsprberry_cl025
	localparam NR = 5; // Number of Receivers 
`else
	localparam NR = 3; // Number of Receivers 
`endif

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
		receiver #(.CICRATE(CICRATE)) receiver_inst(	.clock(clk_ad9866),
														.clock_2x(clk_ad9866_2x),
														.rate(6'd24), 
														.frequency(sync_phase_word[c]),
														.out_strobe(rx_strobe[c]),
														.in_data(adcpipe[c]),
														.out_data_I(rx_I[c]),
														.out_data_Q(rx_Q[c]));
														
		rxFIFO rxFIFO_inst(	.aclr(reset),
							.wrclk(clk_ad9866),.data({rx_I[c], rx_Q[c]}),.wrreq(rx_strobe[c]), .wrempty(emptyFIFO[c]), 
							.rdclk(~spi_ce[0]),.q(rxDataFromFIFO[c]),.rdreq(nnrx==c ? 1'b1 : 1'b0 ));
														
  end
endgenerate

`ifdef wsprberry_cl025
	assign rx_FIFOEmpty = emptyFIFO[0] || emptyFIFO[1] || emptyFIFO[2] || emptyFIFO[3] || emptyFIFO[4]; 
`else
	assign rx_FIFOEmpty = emptyFIFO[0] || emptyFIFO[1] || emptyFIFO[2]; 
`endif	

wire clk_envelope;	
wire clk_internal;															
ad9866pll ad9866pll_inst (.inclk0(clk_76m8), .c0(clk_ad9866), .c1(clk_ad9866_2x), .c2(clk_envelope), .c3(clk_internal),  .locked());
	
wire [11:0] DAC;

reg [11:0] DACDp;
always @ (negedge clk_ad9866) DACDp <= DAC;	
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