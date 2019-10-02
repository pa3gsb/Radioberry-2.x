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


//coeff[0] is available 3 clock cycles after the start pulse

//start: coeff_idx <- 0
//start+1: coeff_idx latched in the ROM address register
//start+2: coeff latched in the ROM output register
//start+3: coeff is available at the output


module qs1r_fir_coeffs(
  input clock,
  input start,
  output signed [23:0] coeff
  );


reg [7:0] coeff_idx;


always @(posedge clock)
  if (start) coeff_idx <= 0; 
  else coeff_idx <= coeff_idx + 8'b1;          


qs1r_fir_coeffs_rom fir_coeffs_rom_inst(
  .clock(clock),
  .address(coeff_idx),
  .q(coeff)
  );



endmodule
