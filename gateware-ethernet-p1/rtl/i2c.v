`timescale 1ns / 1ps

module i2c (
    input  logic         clk,
    input  logic         rst,
    input  logic         init_start,

    // Command slave interface
    input  logic [5:0]   cmd_addr,
    input  logic [31:0]  cmd_data,
    input  logic         cmd_rqst,
    output logic         cmd_ack,   

    /*
     * I2C interface
     */
    input  logic         scl1_i,
    output logic         scl1_o,
    output logic         scl1_t,
    input  logic         sda1_i,
    output logic         sda1_o,
    output logic         sda1_t,    

    input  logic         scl2_i,
    output logic         scl2_o,
    output logic         scl2_t,
    input  logic         sda2_i,
    output logic         sda2_o,
    output logic         sda2_t
);

logic         scl_i, scl_o, scl_t, sda_i, sda_o, sda_t;
logic         en_i2c2, ready;

localparam [2:0]
  STATE_W0    = 3'h0,
  STATE_W1    = 3'h1,
  STATE_W2    = 3'h2,
  STATE_W3    = 3'h3,
  STATE_W4    = 3'h4,
  STATE_W5    = 3'h5,
  STATE_W6    = 3'h6,
  STATE_PASS  = 3'h7;


logic [ 2:0]  state = STATE_W0, state_next;

logic [ 5:0]  icmd_addr;
logic [31:0]  icmd_data;
logic         icmd_rqst;

always @(posedge clk) begin
  state <= state_next;
end 

always @* begin
  state_next = state;

  icmd_addr = cmd_addr;
  icmd_data = cmd_data;
  icmd_rqst = cmd_rqst;

  case(state)
    STATE_W0: begin
      icmd_addr = 6'h3c;
      icmd_data = {8'h06, 1'b1, 7'h6a, 8'h17, 8'h04};
      icmd_rqst = 1'b0;
      if (init_start & ready) begin
          icmd_rqst = 1'b1;
          state_next = STATE_W1;
      end
    end 

    STATE_W1: begin
      icmd_addr = 6'h3c;
      icmd_data = {8'h06, 1'b1, 7'h6a, 8'h18, 8'h40};
      icmd_rqst = 1'b0;
      if (init_start & ready) begin
          icmd_rqst = 1'b1;
          state_next = STATE_W2;
      end
    end 

    STATE_W2: begin
      icmd_addr = 6'h3c;
      icmd_data = {8'h06, 1'b1, 7'h6a, 8'h1e, 8'he8};
      icmd_rqst = 1'b0;
      if (init_start & ready) begin
          icmd_rqst = 1'b1;
          state_next = STATE_W3;
      end
    end 

    STATE_W3: begin
      icmd_addr = 6'h3c;
      icmd_data = {8'h06, 1'b1, 7'h6a, 8'h1f, 8'h80};
      icmd_rqst = 1'b0;
      if (init_start & ready) begin
          icmd_rqst = 1'b1;
          state_next = STATE_W4;
      end
    end     

    STATE_W4: begin
      icmd_addr = 6'h3c;
      icmd_data = {8'h06, 1'b1, 7'h6a, 8'h2d, 8'h01};
      icmd_rqst = 1'b0;
      if (init_start & ready) begin
          icmd_rqst = 1'b1;
          state_next = STATE_W5;
      end
    end 

    STATE_W5: begin
      icmd_addr = 6'h3c;
      icmd_data = {8'h06, 1'b1, 7'h6a, 8'h2e, 8'h10};
      icmd_rqst = 1'b0;
      if (init_start & ready) begin
          icmd_rqst = 1'b1;
          state_next = STATE_W6;
      end
    end 

    STATE_W6: begin
      icmd_addr = 6'h3c;
      icmd_data = {8'h06, 1'b1, 7'h6a, 8'h60, 8'h3b};
      icmd_rqst = 1'b0;
      if (init_start & ready) begin
          icmd_rqst = 1'b1;
          state_next = STATE_PASS;
      end
    end 

    STATE_PASS: begin
      if (~init_start) state_next = STATE_W0;
    end 

  endcase
end

i2c_bus2 i2c_bus2_i (
  .clk(clk),
  .rst(rst),

  .cmd_addr(icmd_addr),
  .cmd_data(icmd_data),
  .cmd_rqst(icmd_rqst),
  .cmd_ack(cmd_ack),  

  .en_i2c2(en_i2c2),
  .ready(ready),

  .scl_i(scl_i),
  .scl_o(scl_o),
  .scl_t(scl_t),
  .sda_i(sda_i),
  .sda_o(sda_o),
  .sda_t(sda_t)
);

assign scl_i = en_i2c2 ? scl2_i : scl1_i;
assign sda_i = en_i2c2 ? sda2_i : sda1_i;

assign scl1_o = scl_o;
assign scl2_o = scl_o;

assign scl1_t = en_i2c2 ? 1'b1 : scl_t;
assign scl2_t = en_i2c2 ? scl_t : 1'b1;

assign sda1_t = en_i2c2 ? 1'b1 : sda_t;
assign sda2_t = en_i2c2 ? sda_t : 1'b1;

endmodule