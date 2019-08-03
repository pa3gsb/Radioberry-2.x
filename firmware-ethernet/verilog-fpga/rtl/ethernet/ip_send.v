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


module ip_send (
  input reset,
  input clock,
  input tx_enable,
  output active,

  input [7:0] data_in,
  output [7:0] data_out,
  input is_icmp,
  input [15:0] length,
  input [31:0] local_ip,
  input [31:0] destination_ip
  );
  
  
  
//-----------------------------------------------------------------------------
//                              ip header
//-----------------------------------------------------------------------------
//icmp or udp code
wire [7:0] protocol_code = is_icmp? 8'd1 : 8'd17;


//packet = header + payload 
wire [15:0] ip_packet_length = 16'd20 + length;
  
  
//ip checksum: 16 bit one's complement of the one's complement sum
wire [19:0] sum = 20'h0C500 + {4'b0, ip_packet_length} + {12'b0, protocol_code} + 
  {4'b0, local_ip[31:16]} + {4'b0, local_ip[15:0]} + 
  {4'b0, destination_ip[31:16]} + {4'b0, destination_ip[15:0]};  
  
wire [16:0] checksum_and_carry = {1'b0, sum[15:0]} + {13'b0, sum[19:16]};
wire [15:0] checksum = ~({15'b0, checksum_and_carry[16]} + checksum_and_carry[15:0]);


//ip header
localparam HDR_LEN = 5'd22;
localparam HI_BIT = HDR_LEN * 8 - 1;
wire [HI_BIT:0] tx_bits = {16'h0800, 16'h4500, ip_packet_length, 40'h0000_0000_80, protocol_code, checksum, local_ip, destination_ip};
reg [4:0] byte_no;


//shift register
reg [HI_BIT:0] shift_reg;
assign data_out = shift_reg[HI_BIT -: 8];




//-----------------------------------------------------------------------------
//                            state machine
//-----------------------------------------------------------------------------
localparam false = 1'b0, true = 1'b1;
reg sending = false;

assign active = tx_enable | sending;

  
always @(posedge clock)  
  begin
  //tx data
  if (active) shift_reg <= {shift_reg[HI_BIT-8:0], data_in};
  else shift_reg <= tx_bits;
  
  //send while payload is coming
  if (tx_enable) begin byte_no <= HDR_LEN - 5'd1; sending <= true; end
  //purge shift register
  else if (byte_no != 0) byte_no <= byte_no - 5'd1;
  //done
  else sending <= false;
  end
  
  
endmodule
  