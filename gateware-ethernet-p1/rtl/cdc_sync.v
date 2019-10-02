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

//  Copyright P Harman VK6APH  2012




// Simple Clock Domain Crossing module that double buffers data
// also simulation version to model CDC issues - in progress

`timescale 1 ns/100 ps

module cdc_sync #(parameter SIZE=1)
  (input  wire [SIZE-1:0] siga,
   input  wire            rstb, clkb, 
   output reg  [SIZE-1:0] sigb);

`ifdef SIMULATION
wire [SIZE-1:0] y1;
reg  [SIZE-1:0] q1a, q1b;
reg  [SIZE-1:0] DLY = 1'b0;

assign y1 = (~DLY & q1a) | (DLY & q1b);

always @(posedge clkb)
begin
  if (rstb)
    {sigb,q1b,q1a} <= '0;
  else
    {sigb,q1b,q1a} <= {y1,q1a,siga};
end
`else // synthesis
reg [SIZE-1:0] q1;

always @(posedge clkb)
begin
  if (rstb)
    {sigb,q1} <= 2'b00;
  else
    {sigb,q1} <= {q1,siga};
end
`endif
endmodule