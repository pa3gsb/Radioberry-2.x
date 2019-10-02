


module rmii_recv (
  input  clk,
  input  clk_div4, 

  output reg [7:0]  phy_rx_data,
  output reg        phy_rx_valid,
  
  //hardware pins
  input  [1:0]      phy_rx,     
  input             phy_dv
);


localparam  IDLE    = 'b000, 
            RECV_10 = 'b001,
            RECV_32 = 'b011,
            RECV_54 = 'b010,
            RECV_76 = 'b100;

logic [2:0] state = IDLE, state_next;
logic [5:0] rx_datad = 6'h00, rx_datad_next;
logic [7:0] rx_data = 8'h00, rx_data_next;
logic [2:0] rx_valid = 3'b000, rx_valid_next;

logic [1:0] phy_rx_d = 2'b00;
logic       phy_dv_d = 1'b0;


always @(negedge clk) begin
  phy_rx_d <= phy_rx;
  phy_dv_d <= phy_dv;
end


// FSM State
always @ (posedge clk) begin
  state <= state_next;
  rx_valid <= rx_valid_next;
  rx_datad <= rx_datad_next;
  rx_data <= rx_data_next;
end

// FSM Combinational
always @* begin

  // Next State
  state_next = state;
  rx_valid_next = (|rx_valid) ? (rx_valid - 3'b001) : rx_valid;
  rx_datad_next = rx_datad;
  rx_data_next = rx_data;

  case (state)
    IDLE: begin
      if (phy_dv_d & (|phy_rx_d)) begin
        rx_datad_next = {rx_datad[5:2],phy_rx_d};
        state_next = RECV_32;
      end
    end 

    RECV_10: begin
      rx_datad_next = {rx_datad[5:2],phy_rx_d};
      state_next = RECV_32;
    end 

    RECV_32: begin
      rx_datad_next = {rx_datad[5:4],phy_rx_d,rx_datad[1:0]};
      state_next = phy_dv_d ? RECV_54 : IDLE;
    end

    RECV_54: begin
      rx_datad_next = {phy_rx_d,rx_datad[3:0]};
      state_next = RECV_76;
    end

    RECV_76: begin
      rx_data_next = {phy_rx_d,rx_datad};
      rx_valid_next = {phy_dv_d,2'b00};
      state_next = phy_dv_d ? RECV_10 : IDLE;
    end
  endcase
end

always @(posedge clk_div4) begin
  phy_rx_data <= rx_data;
  phy_rx_valid <= |rx_valid;
end


endmodule
  