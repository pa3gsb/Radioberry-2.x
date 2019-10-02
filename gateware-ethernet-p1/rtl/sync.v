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
// Steve Haynal KF7O 2018


//-----------------------------------------------------------------------------
// transfer signal to another clock domain
//-----------------------------------------------------------------------------
module sync (
  input clock,
  input sig_in,
  output sig_out
);

parameter DEPTH = 2;

(*preserve*) reg [DEPTH-1:0] sync_chain = {DEPTH{1'b0}};

always @(posedge clock)
  sync_chain <= {sig_in, sync_chain[DEPTH-1:1]};

assign sig_out = sync_chain[0];

endmodule


//-----------------------------------------------------------------------------
// transfer change as pulse to another domain
//-----------------------------------------------------------------------------
module sync_pulse (
  input clock,
  input sig_in,
  output sig_out
);

parameter DEPTH = 3;

(*preserve*) reg [DEPTH-1:0] sync_chain = {DEPTH{1'b0}};

always @(posedge clock)
  sync_chain <= {sig_in, sync_chain[DEPTH-1:1]};

assign sig_out = sync_chain[0] ^ sync_chain[1];

endmodule
