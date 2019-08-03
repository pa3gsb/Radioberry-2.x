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


//  Metis code copyright 2010, 2011, 2012, 2013 Phil Harman VK6APH, Alex Shovkoplyas, VE3NEA.


module udp_send (
  input reset,
  input clock,
  input tx_enable,
  input [7:0] data_in,
  input [15:0] length_in,
  input [15:0] local_port,
  input [15:0] destination_port,
  input [7:0] port_ID,				// determines offset from port base address of 1024
 // input [47:0] remote_mac,
 // input [31:0] remote_ip,
  output active,
  output [7:0] data_out,
  output [15:0] length_out
 // output reg [47:0] destination_mac,
//  output reg [31:0] destination_ip
  );


//udp header
localparam HDR_LEN = 16'd8;
localparam HI_BIT = HDR_LEN * 8 - 1;
assign length_out = HDR_LEN + length_in;
wire [15:0] checksum = 16'b0;						// no checksum needed with IP4
wire [HI_BIT:0] tx_bits = {(local_port + {8'd0,port_ID}), destination_port, length_out, checksum};

//shift reg
reg [HI_BIT:0] shift_reg;
assign data_out = shift_reg[HI_BIT -: 8];
reg [15:0] byte_no;

//state machine
localparam false = 1'b0, true = 1'b1;
reg sending = false;

always @(posedge clock)
  begin
  //tx data
  if (active) begin
	//	destination_mac <= remote_mac;
	//	destination_ip  <= remote_ip;
		shift_reg <= {shift_reg[HI_BIT-8:0], data_in};
  end
  else shift_reg <= tx_bits;
  //send while payload is coming
  if (tx_enable) begin
    byte_no <= length_in + 16'd7; //length_out -16'd1;
    sending <= true; end
  //purge shift register
  else if (byte_no != 0) byte_no <= byte_no - 15'd1;
  //done
  else sending <= false;
  end


assign active = (tx_enable | sending) && (byte_no != 0) ;  //last term prevents one too many bytes being sent



endmodule
