// create a pulse for the rising edge of a signal
`timescale 1 ns/100 ps

module pulsegen (
input wire sig,
input wire rst,
input wire clk,
output wire pulse);

parameter TPD = 0.7;

reg p1;
always @(posedge clk)
begin
  if (rst)
    p1 <= #TPD 1'b0;
  else
    p1 <= #TPD sig; // sig must be synchronous to clk
end

assign pulse = sig & !p1; // one clk wide signal at the rising edge of sig

endmodule