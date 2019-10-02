

module remote_update_app (
  clk,
  rst,
  reconfig
 );


input clk;
input rst;
input reconfig;

logic reset_timer = 1'b0;

always @ (posedge clk) reset_timer <= ~reset_timer;

altera_remote_update_core remote_update_core (
  .read_param  (1'b0), 
  .param       (3'b000),      
  .reconfig    (reconfig),   
  .reset_timer (reset_timer),
  .clock       (clk),      
  .reset       (rst),      
  .busy        (),       
  .data_out    (),   
  .read_source (2'b00),
  .write_param (1'b0),
  .data_in     (24'h000000),    
  .ctl_nupdt   (1'b0)        
);

endmodule
