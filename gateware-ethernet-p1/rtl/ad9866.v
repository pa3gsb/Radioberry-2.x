//
//  Hermes Lite
// 
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// (C) Steve Haynal KF7O 2014-2018

module ad9866 (
  clk,
  clk_2x,

  tx_data,
  rx_data,
  tx_en,

  rxclip,
  rxgoodlvl,
  rxclrstatus,

  rffe_ad9866_tx,
  rffe_ad9866_rx,
  rffe_ad9866_rxsync,
  rffe_ad9866_rxclk,  
  rffe_ad9866_txquiet_n,
  rffe_ad9866_txsync,
  rffe_ad9866_mode
);

input             clk;
input             clk_2x;

input   [11:0]    tx_data;
output  [11:0]    rx_data;
input             tx_en;

output logic      rxclip = 1'b0;
output logic      rxgoodlvl = 1'b0;
input             rxclrstatus;

`ifdef HALFDUPLEX
inout   [5:0]     rffe_ad9866_tx;
inout   [5:0]     rffe_ad9866_rx;
output            rffe_ad9866_rxsync;
output            rffe_ad9866_rxclk; 
`else
output  [5:0]     rffe_ad9866_tx;
input   [5:0]     rffe_ad9866_rx;
input             rffe_ad9866_rxsync;
input             rffe_ad9866_rxclk; 
`endif
output            rffe_ad9866_txquiet_n;
output            rffe_ad9866_txsync;

output            rffe_ad9866_mode;


// TX Path
logic   [11:0]    tx_data_d1;
logic             tx_sync;
logic             tx_en_d1;

// RX Path
logic   [11:0]    rx_data_assemble;
logic    [5:0]    rffe_ad9866_rx_d1, rffe_ad9866_rx_d2;
logic             rffe_ad9866_rxsync_d1;


logic             rxclipp, rxclipn;
logic             rxgoodlvlp, rxgoodlvln;



// TX Path

`ifdef HALFDUPLEX
always @(posedge clk) tx_en_d1 <= tx_en;

always @(posedge clk) tx_data_d1 <= tx_data;
assign rffe_ad9866_tx = tx_en_d1 ? tx_data_d1[11:6] : 6'bZ;
assign rffe_ad9866_rx = tx_en_d1 ? tx_data_d1[5:0]  : 6'bZ;
assign rffe_ad9866_txsync = tx_en_d1;
assign rffe_ad9866_txquiet_n = clk;

`else
always @(posedge clk_2x) begin
  tx_en_d1 <= tx_en;
  tx_sync <= ~tx_sync;
  if (tx_en_d1) begin
    if (tx_sync) begin 
      tx_data_d1 <= tx_data;
      rffe_ad9866_tx <= tx_data_d1[5:0];
    end else begin
      rffe_ad9866_tx <= tx_data_d1[11:6];
    end
    rffe_ad9866_txsync <= tx_sync;
  end else begin
    rffe_ad9866_tx <= 6'h00;
    rffe_ad9866_txsync <= 1'b0;
  end
end

assign rffe_ad9866_txquiet_n = tx_en_d1; 

`endif

// RX Path

`ifdef HALFDUPLEX
always @(posedge clk) rx_data_assemble <= {rffe_ad9866_tx,rffe_ad9866_rx};
assign rffe_ad9866_rxsync = ~tx_en_d1;
assign rffe_ad9866_rxclk = clk;
assign rffe_ad9866_mode = 1'b0;

`else
// Assume that ad9866_rxclk is synchronous to ad9866clk
// Don't know the phase relation
always @(posedge clk_2x) begin
  rffe_ad9866_rx_d1 <= rffe_ad9866_rx;
  rffe_ad9866_rx_d2 <= rffe_ad9866_rx_d1;
  rffe_ad9866_rxsync_d1 <= rffe_ad9866_rxsync;
  if (rffe_ad9866_rxsync_d1) rx_data_assemble <= {rffe_ad9866_rx_d2,rffe_ad9866_rx_d1};
end
assign rffe_ad9866_mode = 1'b1;
`endif

always @ (posedge clk) rx_data <= rx_data_assemble;


assign rxclipp = (rx_data == 12'b011111111111);
assign rxclipn = (rx_data == 12'b100000000000);

// Like above but 2**11.585 = (4096-1024) = 3072
assign rxgoodlvlp = (rx_data[11:9] == 3'b011);
assign rxgoodlvln = (rx_data[11:9] == 3'b100);

always @(posedge clk) begin
  if (rxclrstatus) rxclip <= 1'b0;
  if (rxclipp | rxclipn) rxclip <= 1'b1;
end

always @(posedge clk) begin
  if (rxclrstatus) rxgoodlvl <= 1'b0;
  if (rxgoodlvlp | rxgoodlvln) rxgoodlvl <= 1'b1;
end

endmodule // ad9866

