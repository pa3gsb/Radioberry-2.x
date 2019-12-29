//
//  Radioberry
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

// (C) Johan Maas 2017
//
//	Filters for a setup like created in :
//	https://github.com/W9JSW/Hermes-Lite/tree/Amplifiers/frontend/v2-Amp
//
//	and used by Jacinto CU2ED
//
//	
//	 160m 	  	BAND0 = 7'b0000001 
//	 80/75m	  	BAND1 = 7'b0000010	
//	 60/40m  	BAND2 = 7'b0000100	
//	 30/20m  	BAND3 = 7'b0001000	
//	 17m  		BAND4 = 7'b0010000	
//	 15m		BAND5 = 7'b0100000	
//	 12/10m		BAND6 = 7'b1000000 

module filter(clock, frequency, selected_filter);
input  wire        clock;
input  wire [31:0] frequency;	
output reg   [6:0] selected_filter;
		
always @(posedge clock)  
begin 
	if      (frequency > 24000000) selected_filter <= 7'b0000001;  //BAND6	
	else if (frequency > 20000000) selected_filter <= 7'b0000010;  //BAND5	  
	else if (frequency > 16500000) selected_filter <= 7'b0000100;  //BAND4	
	else if (frequency > 8000000)  selected_filter <= 7'b0001000;  //BAND3	
	else if (frequency > 5000000)  selected_filter <= 7'b0010000;  //BAND2
	else if (frequency > 2500000)  selected_filter <= 7'b0100000;  //BAND1
	else 	selected_filter <= 7'b1000000 ; 	// BAND0
end 
endmodule
