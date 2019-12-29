
`timescale 1us/1ns

module nco1 (
  clk,
  rst,
  phi,
  cos,
  sin
);

input             clk;
input             rst;
input     [31:0]  phi;
output    [18:0]  sin;
output    [18:0]  cos;

logic [31:0]      angle = 32'h00;

parameter         CALCTYPE = 0;

always @(posedge clk) begin
  if (rst) angle <= 32'h00;
  else angle <= angle + phi;
end


sincos #(.CALCTYPE(CALCTYPE)) sincos_i (
  .clk(clk),
  .angle(angle[31:12]),
  .cos(cos),
  .sin(sin)
);

endmodule 


