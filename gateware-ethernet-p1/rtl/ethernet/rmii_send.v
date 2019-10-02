
module rmii_send (
  input       clk,
  input       clk_div4,
  input [7:0] phy_tx_data,
  input       phy_tx_valid,

  //hardware pins
  output reg [1:0]  phy_tx = 2'b00,
  output reg        phy_tx_en = 1'b0
);


localparam  SEND_10 = 'b00,
            SEND_32 = 'b01,
            SEND_54 = 'b11,
            SEND_76 = 'b10;

logic [1:0] state = SEND_10, state_next;
logic [1:0] phy_tx_next;
logic       phy_tx_en_next;

logic [7:0]     phy_tx_data_d1 = 8'h00;
logic           phy_tx_valid_d1 = 1'b0;

logic  [1:0]    phy_tx_i = 2'b00;
logic           phy_tx_en_i = 1'b0;


// FSM State
always @ (negedge clk) begin
  state <= state_next;
  phy_tx_en_i <= phy_tx_en_next;
  phy_tx_i <= phy_tx_next;
end

// FSM Combinational
always @* begin

  // Next State
  state_next = state;
  phy_tx_en_next = 1'b0;
  phy_tx_next = phy_tx_i;

  case (state)
    SEND_10: begin
      if (phy_tx_valid_d1) begin
        phy_tx_en_next = 1'b1;
        phy_tx_next = phy_tx_data_d1[1:0];
        state_next = SEND_32;
      end
    end 

    SEND_32: begin
      phy_tx_en_next = 1'b1;
      phy_tx_next = phy_tx_data_d1[3:2];
      state_next = SEND_54;
    end

    SEND_54: begin
      phy_tx_en_next = 1'b1;
      phy_tx_next = phy_tx_data_d1[5:4];
      state_next = SEND_76;
    end

    SEND_76: begin
      phy_tx_en_next = 1'b1;
      phy_tx_next = phy_tx_data_d1[7:6];
      state_next = SEND_10;
    end
  endcase
end

always @(posedge clk) begin
  phy_tx_en <= phy_tx_en_i;
  phy_tx <= (phy_tx_en_i) ? phy_tx_i : 2'b00;
end

always @(posedge clk_div4) begin
  phy_tx_data_d1 <= phy_tx_data;
  phy_tx_valid_d1 <= phy_tx_valid;
end



endmodule
