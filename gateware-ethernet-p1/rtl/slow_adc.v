
module slow_adc(
  input  logic         clk,
  input  logic         rst,

  output logic [11:0]  ain0,
  output logic [11:0]  ain1,
  output logic [11:0]  ain2,
  output logic [11:0]  ain3,

  input  logic         scl_i,
  output logic         scl_o,
  output logic         scl_t,
  input  logic         sda_i,
  output logic         sda_o,
  output logic         sda_t
);

logic [6:0]  cmd_address;
logic        cmd_start;
logic        cmd_read;
logic        cmd_write;
logic        cmd_write_multiple;
logic        cmd_stop;
logic        cmd_valid;
logic        cmd_ready;

logic [7:0]  data_in;
logic        data_in_valid;
logic        data_in_ready;
logic        data_in_last;

logic [7:0]  data_out;
logic        data_out_valid;
logic        data_out_ready;
logic        data_out_last;

logic [3:0]  state, next_state;
logic [3:0]  msbnibble;

i2c_master i2c_master_i (
  .clk(clk),
  .rst(rst),
  /*
   * Host interface
   */
  .cmd_address(cmd_address),
  .cmd_start(cmd_start),
  .cmd_read(cmd_read),
  .cmd_write(cmd_write),
  .cmd_write_multiple(cmd_write_multiple),
  .cmd_stop(cmd_stop),
  .cmd_valid(cmd_valid),
  .cmd_ready(cmd_ready),

  .data_in(data_in),
  .data_in_valid(data_in_valid),
  .data_in_ready(data_in_ready),
  .data_in_last(data_in_last),

  .data_out(data_out),
  .data_out_valid(data_out_valid),
  .data_out_ready(data_out_ready),
  .data_out_last(data_out_last),

  /*
   * I2C interface
   */
  .scl_i(scl_i),
  .scl_o(scl_o),
  .scl_t(scl_t),
  .sda_i(sda_i),
  .sda_o(sda_o),
  .sda_t(sda_t),

  /*
   * Status
   */
  .busy(),
  .bus_control(),
  .bus_active(),
  .missed_ack(),

  /*
   * Configuration
   */
  .prescale(16'h0002),
  .stop_on_idle(1'b0)
);



// Control
localparam [3:0]
  READ0  = 4'h0,
  READ1  = 4'h1,
  READ2  = 4'h2,
  READ3  = 4'h3,
  READ4  = 4'h4,
  READ5  = 4'h5,
  READ6  = 4'h6,
  READ7  = 4'h7,
  WRITE0 = 4'h8,
  WRITE1 = 4'h9;

always @(posedge clk) begin
  if (rst) begin
    state <= WRITE0;
  end else begin
    state <= next_state;
  end
end

assign cmd_address = 7'h34;
assign cmd_write_multiple = 1'b0;
assign cmd_start = 1'b0;
assign data_in = 8'h07;
assign data_out_ready = 1'b1;
assign data_in_last = 1'b1;

always @* begin
  next_state = state;

  cmd_valid = 1'b1; 
  cmd_read = 1'b1;
  cmd_write = 1'b0;
  cmd_stop = 1'b0;

  data_in_valid = 1'b0;

  case(state)

    WRITE0: begin
      cmd_read = 1'b0;
      cmd_write = 1'b1;
      cmd_stop = 1'b1;
      if (cmd_ready) next_state = WRITE1;
    end

    WRITE1: begin
      data_in_valid = 1'b1;
      if (data_in_ready) next_state = READ0;
    end

    READ0: if (data_out_valid) next_state = READ1;
    READ1: if (data_out_valid) next_state = READ2;
    READ2: if (data_out_valid) next_state = READ3;
    READ3: if (data_out_valid) next_state = READ4;
    READ4: if (data_out_valid) next_state = READ5;
    READ5: if (data_out_valid) next_state = READ6;
    READ6: if (data_out_valid) next_state = READ7;

    READ7: begin
      cmd_stop = 1'b1;
      if (data_out_valid) next_state = WRITE0;
    end
  endcase
end


always @(posedge clk) begin
  if (data_out_valid) begin
    if (~state[0]) msbnibble <= data_out[3:0];
      
    case(state)
      READ1: ain0 <= {msbnibble,data_out};
      READ3: ain1 <= {msbnibble,data_out};
      READ5: ain2 <= {msbnibble,data_out};
      READ7: ain3 <= {msbnibble,data_out};
    endcase
  end
end

endmodule


