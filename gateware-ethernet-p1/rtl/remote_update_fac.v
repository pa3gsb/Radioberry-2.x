

module remote_update_fac (
  clk,
  rst,
  bootapp
 );

input clk;
input rst;
input bootapp;

logic reconfig = 1'b0;
logic busy;
logic write;

logic [2:0] param;
logic [23:0] data_in;



localparam START      = 'b000,
           ANF        = 'b001,
           ANF_BUSY   = 'b011,
           WRITE      = 'b010,
           WRITE_BUSY = 'b110,
           REBOOT     = 'b100,
           DONE       = 'b101;

logic   [ 2:0]  state = START;
logic   [ 2:0]  state_next;


always @ (posedge clk) begin
  if (rst) begin
    state <= START;
  end else begin
    state <= state_next;
  end 
end

// FSM Combinational
always @* begin

  // Next State
  state_next = state;

  // Combinational output
  write = 1'b0;
  reconfig = 1'b0;
  param = 3'b000;
  data_in = 24'h000000;


  case (state)
    START: begin
      if (~busy) state_next = ANF;
      param = 3'b101;
      data_in = 24'h01;
    end

    ANF: begin
      write = 1'b1;
      param = 3'b101;
      data_in = 24'h01;
      if (busy) state_next = ANF_BUSY;
    end

    ANF_BUSY: begin
      param = 3'b100;
      data_in = 24'h040000;
      if (~busy) state_next = WRITE;
    end


    WRITE: begin
      write = 1'b1;
      param = 3'b100;
      data_in = 24'h040000;
      if (busy) state_next = WRITE_BUSY;
    end

    WRITE_BUSY: begin
      if (~busy) state_next = REBOOT;
    end

    REBOOT: begin
      reconfig = bootapp;
      //if (busy) state_next = DONE;
      state_next = DONE;
    end

    DONE: begin
      state_next = DONE;
    end

    default: begin
      state_next = START;
    end

  endcase
end



altera_remote_update_core remote_update_core (
  .read_param  (1'b0), 
  .param       (param),      
  .reconfig    (reconfig),   
  .reset_timer (1'b0),
  .clock       (clk),      
  .reset       (rst),      
  .busy        (busy),       
  .data_out    (),   
  .read_source (2'b00),
  .write_param (write),
  .data_in     (data_in),    
  .ctl_nupdt   (1'b0)        
);

endmodule
