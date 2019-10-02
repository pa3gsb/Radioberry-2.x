`timescale 1ns / 1ps

module i2c_bus2
(
  clk,
  rst,

  cmd_addr,
  cmd_data,
  cmd_rqst,
  cmd_ack,

  en_i2c2,
  ready,

  scl_i,
  scl_o,
  scl_t,
  sda_i,
  sda_o,
  sda_t
);

input         clk;
input         rst;

// Command slave interface
input  [5:0]  cmd_addr;
input  [31:0] cmd_data;
input         cmd_rqst;
output        cmd_ack;

output  logic en_i2c2;
output        ready;    
 
input         scl_i;
output        scl_o;
output        scl_t;
input         sda_i;
output        sda_o;
output        sda_t;

logic [6:0]   cmd_address;
logic         cmd_start;
logic         cmd_read;
logic         cmd_write;
logic         cmd_write_multiple;
logic         cmd_stop;
logic         cmd_valid;
logic         cmd_ready;

logic [7:0]   data_in;
logic         data_in_valid;
logic         data_in_ready;
logic         data_in_last;

logic [7:0]   data_out;
logic         data_out_valid;
logic         data_out_ready;
logic         data_out_last;

logic [2:0]   state, state_next;
logic         busy, missed_ack;

logic [6:0]   cmd_reg, cmd_next;
logic [7:0]   data0_reg, data0_next, data1_reg, data1_next;

logic         cmd_ack_reg, cmd_ack_next;

logic [6:0]   filter_select_reg, filter_select_next;

logic         en_i2c2_next;




// Control
localparam [2:0]
  STATE_IDLE          = 3'h0,
  STATE_CMDADDR       = 3'h1,
  STATE_WRITE_DATA0   = 3'h2,
  STATE_WRITE_DATA1   = 3'h3,
  STATE_FCMDADDR      = 3'h5,
  STATE_WRITE_FDATA0  = 3'h6,
  STATE_WRITE_FDATA1  = 3'h7,
  STATE_WRITE_FDATA2  = 3'h4;



always @(posedge clk) begin
  if (rst) begin
    state <= STATE_IDLE;
    cmd_reg <= 'h0;
    data0_reg <= 'h0;
    data1_reg <= 'h0;
    cmd_ack_reg <= 1'b0;
    filter_select_reg <= 'h0;
  end else begin
    state <= state_next;
    cmd_reg <= cmd_next;
    data0_reg <= data0_next;
    data1_reg <= data1_next;
    cmd_ack_reg <= cmd_ack_next;
    filter_select_reg <= filter_select_next;
    en_i2c2 <= en_i2c2_next;
  end
end

assign cmd_address = cmd_reg;
assign cmd_start = 1'b0;
assign cmd_read = 1'b0;
assign cmd_write_multiple = 1'b0;
assign cmd_stop = 1'b0;

assign data_in_last = 1'b1;

assign data_out_ready = 1'b1;

assign cmd_ack = cmd_ack_reg;


always @* begin
  state_next = state;

  cmd_valid = 1'b0; 
  cmd_write = 1'b0;
  //cmd_stop = 1'b0;
  cmd_ack_next = cmd_ack;

  data_in = data0_reg;
  data_in_valid = 1'b0;

  filter_select_next = filter_select_reg;

  cmd_next = cmd_reg;
  data0_next = data0_reg;
  data1_next = data1_reg; 
  en_i2c2_next = en_i2c2;

  ready = 1'b0;
  
  case(state)

    STATE_IDLE: begin
      ready = ~busy;
      if (cmd_rqst) begin
        cmd_ack_next = 1'b1;
        if (((cmd_addr == 6'h3d) | (cmd_addr == 6'h3c)) & (cmd_data[31:24] == 8'h06)) begin
          // Must send
          if (~busy) begin
            cmd_next = cmd_data[22:16];
            data0_next  = cmd_data[15:8];
            data1_next = cmd_data[7:0];
            en_i2c2_next = (cmd_addr == 6'h3d);
            state_next = STATE_CMDADDR;
          end else begin
            cmd_ack_next = 1'b0; // Missed
          end
        end

        // Filter select update
        if (cmd_addr == 6'h00) begin
          if (cmd_data[23:17] != filter_select_reg) begin
            // Must send
            if (~busy) begin
              filter_select_next = cmd_data[23:17];
              cmd_next = 'h20;
              data0_next = 'h0a;
              data1_next = {1'b0,cmd_data[23:17]};
              en_i2c2_next = 1'b1;
              state_next = STATE_FCMDADDR;
            end else begin
              cmd_ack_next = 1'b0; // Missed
            end
          end
        end
      end
    end

    STATE_CMDADDR: begin
      cmd_valid = 1'b1;
      cmd_write = 1'b1;
      if (cmd_ready) state_next = STATE_WRITE_DATA0;
    end

    STATE_WRITE_DATA0: begin
      cmd_valid = 1'b1;
      cmd_write = 1'b1;
      data_in_valid = 1'b1;
      data_in = data0_reg;
      if (data_in_ready) state_next = STATE_WRITE_DATA1;
    end

    STATE_WRITE_DATA1: begin
      cmd_valid = 1'b1;
      cmd_write = 1'b1;
      data_in_valid = 1'b1;
      data_in = data1_reg;
      if (data_in_ready) state_next = STATE_IDLE;
    end

    STATE_FCMDADDR: begin
      cmd_valid = 1'b1;
      cmd_write = 1'b1;
      if (cmd_ready) state_next = STATE_WRITE_FDATA0;
    end

    STATE_WRITE_FDATA0: begin
      cmd_valid = 1'b1;
      cmd_write = 1'b1;
      data_in_valid = 1'b1;
      data_in = data0_reg;
      if (data_in_ready) state_next = STATE_WRITE_FDATA1;
    end

    STATE_WRITE_FDATA1: begin
      cmd_valid = 1'b1;
      cmd_write = 1'b1;
      data_in_valid = 1'b1;
      data_in = data1_reg;
      if (data_in_ready) state_next = STATE_WRITE_FDATA2;
    end

    STATE_WRITE_FDATA2: begin
      cmd_valid = 1'b1;
      cmd_write = 1'b1;
      data_in_valid = 1'b1;
      data_in = 'h00;
      if (data_in_ready) state_next = STATE_IDLE;
    end

  endcase
end


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

  // I2C
  .scl_i(scl_i),
  .scl_o(scl_o),
  .scl_t(scl_t),
  .sda_i(sda_i),
  .sda_o(sda_o),
  .sda_t(sda_t),

  // Status
  .busy(busy),
  .bus_control(),
  .bus_active(),
  .missed_ack(missed_ack),

  // Configuration
  .prescale(16'h0002),
  .stop_on_idle(1'b1)
);

endmodule


