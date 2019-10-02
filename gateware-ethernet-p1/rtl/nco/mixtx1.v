
`timescale 1us/1ns

module mixtx1 (
  clk,
  rst,
  phi,
  i_sig,
  q_sig,
  i_result,
  q_result
);

input                 clk;
input                 rst;
input         [31:0]  phi;
input  signed [17:0]  i_sig;
input  signed [17:0]  q_sig;
output signed [12:0]  i_result;
output signed [12:0]  q_result;

logic         [18:0]  sin, ssin;
logic         [18:0]  cos, scos;

logic  signed [17:0]  ssin_q, scos_q;
logic  signed [35:0]  qa_data_d, qb_data_d;
logic  signed [19:0]  q_data_d;
logic  signed [35:0]  ia_data_d, ib_data_d;
logic  signed [19:0]  i_data_d;
logic  signed [12:0]  i_rounded, q_rounded;

nco1 #(.CALCTYPE(3)) nco1_i (
  .clk(clk),
  .rst(rst),
  .phi(phi),
  .cos(cos),
  .sin(sin)
);

assign ssin = {sin[18],~sin[17:0]} + 19'h01;
assign scos = {cos[18],~cos[17:0]} + 19'h01;

always @(posedge clk) begin
  ssin_q <= sin[18] ? ssin[18:1] : sin[18:1];
  scos_q <= cos[18] ? scos[18:1] : cos[18:1];
end

always @(posedge clk) begin
  qa_data_d <= $signed(i_sig) * ssin_q;
  qb_data_d <= $signed(q_sig) * scos_q;
end

always @(posedge clk) begin
  ia_data_d <= $signed(i_sig) * scos_q;
  ib_data_d <= $signed(q_sig) * ssin_q;
end

assign i_data_d = ia_data_d[35:16] - ib_data_d[35:16];
assign q_data_d = qa_data_d[35:16] + qb_data_d[35:16];

//always @(posedge clk) begin
//  i_rounded <= i_data_d[19:2] + {17'h00,i_data_d[1]};
//  q_rounded <= q_data_d[19:2] + {17'h00,q_data_d[1]};
//end

//assign i_result = i_data_d[19:8];
//assign q_result = q_data_d[19:8];

//assign i_result = i_rounded[17:6];
//assign q_result = q_rounded[17:6];

always @(posedge clk) begin
  i_rounded <= {i_data_d[19],i_data_d[19:8]} + {12'h00,i_data_d[7]};
  q_rounded <= {q_data_d[19],q_data_d[19:8]} + {12'h00,q_data_d[7]};
end

assign i_result = i_rounded;
assign q_result = q_rounded;


endmodule 


