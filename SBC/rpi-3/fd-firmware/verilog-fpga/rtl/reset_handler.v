// Project:   RXSDR
//
// Design:    Reset handler
// Module:    reset_handler

// Johan Maas PA3GSB 
// Date:    8 December 2010

`include "timescale.v"


module reset_handler(clock, reset);

input clock;
output reset;

reg reset;

reg [23:0] reset_counter;

always @(posedge clock)
begin
		if (reset_counter > 24'h0F0000)
			reset <= 1'b0;	
	   else begin
			reset <= 1'b1;	
			reset_counter <= reset_counter +1;
		end
		
		
end


initial
begin
  reset_counter = 24'h000000;
  reset = 1'b1;
  $display ("Reset handler initialization done");
end

endmodule
