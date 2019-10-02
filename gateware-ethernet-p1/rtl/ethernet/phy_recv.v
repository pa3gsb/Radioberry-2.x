//-----------------------------------------------------------------------------
//                    Copyright (c) 2012 HPSDR Team
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// demultiplex phy nibbles, produce clock signal depending on speed
//-----------------------------------------------------------------------------


module phy_recv (

  //receive: data and active are valid at posedge of clock
  input         clock, 
  input  [7:0]  phy_rx_data,
  input         phy_rx_valid,

  output active
);
 
//-----------------------------------------------------------------------------
//                          preamble detector
//-----------------------------------------------------------------------------
localparam MIN_PREAMBLE_LENGTH = 3'd5;


reg [2:0] preamble_cnt;
reg payload_coming = 0;


always @(posedge clock) 
  //RX-DV low, nothing is being received
  if (!phy_rx_valid) begin payload_coming <= 1'b0; preamble_cnt <= MIN_PREAMBLE_LENGTH; end
  //RX-DV high, but payload is not being received yet
  else if (!payload_coming) 
    //count preamble bytes
    if (phy_rx_data == 8'h55) begin if (preamble_cnt != 0) preamble_cnt <= preamble_cnt - 3'd1; end
    //enough preamble bytes plus SFD, payload follows
    else if ((preamble_cnt == 0) && (phy_rx_data == 8'hD5)) payload_coming <= 1'b1;
    //wrong byte received, reset preamble byte count
    else preamble_cnt <= MIN_PREAMBLE_LENGTH;
      
      
assign active = phy_rx_valid & payload_coming;
      

  
endmodule
  