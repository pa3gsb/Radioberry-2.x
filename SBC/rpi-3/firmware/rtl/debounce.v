//
//  HPSDR - High Performance Software Defined Radio
//
//  Hermes code. 
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

// (C) Phil Harman VK6APH 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 


// Push Button debounce routine
// Found on the Internet, author unknown
// used to debounce a switch or push button.
// Input is pb and outout clean_pb.
// Button must be stable for debounce time before state changes,
// debounce time is dependent on clk and counter_bits

// eg with clock = 24MHz and counter_bits = 18 stable time is 

//  (10^-6/24) * 2^18 = 11mS

//  Phil Harman VK6APH 15th February 2006

module debounce(clean_pb, pb, clk);
	
	output clean_pb;
	input pb, clk;
	
parameter counter_bits = 18;

reg [counter_bits:0] count;
reg [3:0] pb_history;
reg clean_pb;

always @ (posedge clk)
begin
	pb_history <= {pb_history[2:0], pb};
	
	if (pb_history[3] != pb_history[2])
		count <= 1'b0;
	else if(count[counter_bits])
		clean_pb <= pb_history[3];
	else
		count <= count + 1'b1;
end 
endmodule