//
//  HPSDR - High Performance Software Defined Radio
//
//  Metis code.
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


//  Metis code copyright 2010, 2011, 2012, 2013 Alex Shovkoplyas, VE3NEA.


module phy_send (
  input [7:0] data,
  input tx_enable,
  output active,
  input clock,

  output [7:0]  phy_tx_data,
  output        phy_tx_valid
);



//-----------------------------------------------------------------------------
//                            shift reg
//-----------------------------------------------------------------------------
localparam PREAMBLE_BYTES = 64'h55555555555555D5;
localparam PREAMB_LEN = 4'd8;
localparam HI_BIT = 8*PREAMB_LEN - 1;
reg [HI_BIT:0] shift_reg;
reg [3:0] bytes_left;


//-----------------------------------------------------------------------------
//                           state machine
//-----------------------------------------------------------------------------
localparam ST_IDLE = 1, ST_SEND = 2, ST_GAP = 4;
reg [2:0] state = ST_IDLE;


wire sending = tx_enable | (state == ST_SEND);
assign active = sending | (state == ST_GAP);


always @(posedge clock)
  begin
  if (sending) shift_reg <= {shift_reg[HI_BIT-8:0], data};
  else shift_reg <= PREAMBLE_BYTES;

  case (state)
    ST_IDLE:
      //receiving the first payload byte
      if (tx_enable) state <= ST_SEND;

    ST_SEND:
      //receiving payload data
      if (tx_enable) bytes_left <= PREAMB_LEN - 4'd1;
      //purging shift register
      else if (bytes_left != 0) bytes_left <= bytes_left - 4'd1;
      //starting inter-frame gap
      else begin bytes_left <= 4'd12; state <= ST_GAP; end

    ST_GAP:
      if (bytes_left != 0) bytes_left <= bytes_left - 4'd1;
      else state <= ST_IDLE;
    endcase
  end


assign phy_tx_data = shift_reg[HI_BIT -: 8];
assign phy_tx_valid = sending;


endmodule
