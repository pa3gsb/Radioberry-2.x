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


//  Metis code copyright 2013 Alex Shovkoplyas, VE3NEA.



//-----------------------------------------------------------------------------
//                         calculation of crc32 
//-----------------------------------------------------------------------------
module crc32 (
  input clock,
  input clear,
  input enable,
  input [7:0] data,
  output [31:0] result
  );
  
  
wire [511:0] crc_table = {
  32'h4DBDF21C, 32'h500AE278, 32'h76D3D2D4, 32'h6B64C2B0,
  32'h3B61B38C, 32'h26D6A3E8, 32'h000F9344, 32'h1DB88320,
  32'hA005713C, 32'hBDB26158, 32'h9B6B51F4, 32'h86DC4190,
  32'hD6D930AC, 32'hCB6E20C8, 32'hEDB71064, 32'hF0000000};
  
  
reg [31:0] prev_result;


//process low nibble, then high nibble
wire [31:0] crc_lo = prev_result[31:4] ^ crc_table[511-32*(prev_result[3:0] ^ data[3:0]) -: 32]; 
wire [31:0] crc_hi = crc_lo[31:4] ^ crc_table[511-32*(crc_lo[3:0] ^ data[7:4]) -: 32]; 
  

//clear, compute, or hold  
always @(posedge clock)
  if (clear) prev_result <= 32'b0;
  else if (enable) prev_result <= crc_hi;


assign result = crc_hi;



endmodule
