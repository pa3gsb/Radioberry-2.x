// 
// radioberry rxstream; from radioberry to computer. 
//
// only rx-iq stream (control via SPI)
// the rx fifo contains the 24 bit I or Q data; these 3 bytes must be placed byte by byte in the ethernet packet. 
//
//
// going for ethernet packet length 1026. 6 bytes per IQ sample.
// 342 samples makes 171 IQ samples per ethernet packet.

module rxstream(
clk,

udp_tx_enable,
udp_tx_request,
udp_tx_data,
udp_tx_length,

rx_data,
rx_request,
rx_length
);

input               clk;

input               udp_tx_enable;
output              udp_tx_request;
output [7:0]        udp_tx_data;
output logic [10:0] udp_tx_length = 'd1026;


input [23:0]        rx_data;
output              rx_request;
input [10:0]        rx_length;



localparam START        = 4'h0,
           RXDATA2      = 4'h1,
           RXDATA1      = 4'h2,
           RXDATA0      = 4'h3;

logic   [ 3:0]  state = START;
logic   [ 3:0]  state_next;

logic   [10:0]  byte_no = 11'h00;
logic   [10:0]  byte_no_next; 

logic   [ 7:0]  udp_data = 'd0, udp_data_next;


// State
always @ (posedge clk) begin
	state <= state_next;
	byte_no <= byte_no_next;
	udp_data <= udp_data_next;
end


// FSM Combinational
always @* begin

  state_next = state;
  byte_no_next = byte_no;
  udp_data_next = udp_data;
  
  udp_tx_request = 1'b0;
  
  rx_request = 1'b0;
  
	case (state)
		START: begin
			if (rx_length > 11'd342) begin
				byte_no_next = 'd1026;
				udp_tx_request = 1'b1;
				if (udp_tx_enable) state_next = RXDATA2;
			end
		end
		
		RXDATA2: begin
			byte_no_next = byte_no - 11'd1;
			udp_data_next = rx_data[23:16];
		
			state_next = RXDATA1;
		end      

		RXDATA1: begin
			byte_no_next = byte_no - 11'd1;
			udp_data_next = rx_data[15:8];

			state_next = RXDATA0;        
		end  

		RXDATA0: begin
		  byte_no_next = byte_no - 11'd1;
		  udp_data_next = rx_data[7:0];
		  
		  rx_request = 1'b1; // Pop next word

		  state_next = (&byte_no) ? RXDATA2 : START;
		end 		
	
		default: state_next = START;

	endcase // state
  
end // always @*

endmodule