/*
--------------------------------------------------------------------------------
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
Boston, MA  02110-1301, USA.
--------------------------------------------------------------------------------
*/


//------------------------------------------------------------------------------
//           Copyright (c) 2008 Alex Shovkoplyas, VE3NEA
//------------------------------------------------------------------------------


//multiply-accumulate 256 24-bit signed values
//output width = 24*2 + Log2(256) = 56 bit


module qs1r_fir_mac(
  input clock,
  input clear,
  input signed [23:0] in_data_1,
  input signed [23:0] in_data_2,
  output reg signed [55:0] out_data
  );
wire signed [47:0] product;

always @(posedge clock)
  if (clear) out_data <= 0;
  else out_data <= out_data + product;


//pipelined multiplier: throughput = 1, latency = 3
qs1r_mult_24Sx24S mult_24Sx24S_inst(
  .aclr (clear),
  .clock (clock),
  .dataa (in_data_1),
  .datab (in_data_2),
  .result (product)
  );

endmodule
