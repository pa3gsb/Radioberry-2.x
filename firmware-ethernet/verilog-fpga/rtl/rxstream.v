// 
// radioberry rxstream; placing the RX IQ samples into UDP stream; stream from radioberry to computer for further DSP processing. 
//
// only rx-iq stream (control via SPI) 
// The rx fifo contains the 24 bit I or Q data; these 3 bytes must be placed byte by byte in the UDP packet. 
//
//
// We are going here for udp payload length of 1026 + 4 bytes for UDP packet seq number.
// 6 bytes per IQ sample.
// 1026 / 6 =>  171 IQ samples per UDP packet.

module rxstream(
clk,

run,

have_ip,

udp_tx_enable,
udp_tx_request,
udp_tx_data,
udp_tx_length,

rx_data,
rx_request,
rx_length

);

input               clk;
input 				run;

input 				have_ip;

input               udp_tx_enable;
output logic        udp_tx_request;
output logic [7:0]  udp_tx_data;
output logic [10:0] udp_tx_length = 11'd1030;  


input [47:0]        rx_data;
output  logic       rx_request;
input [9:0]        	rx_length;



localparam 	INIT 	  = 4'h0,
			START     = 4'h2,
			SEQ_NR	  = 4'h3,
			I_RXDATA2 = 4'h4,
			I_RXDATA1 = 4'h5,
			I_RXDATA0 = 4'h6,
			Q_RXDATA2 = 4'h7,
			Q_RXDATA1 = 4'h8,
			Q_RXDATA0 = 4'h9;

logic   [ 3:0]  state;
logic   [ 3:0]  state_next;

logic   [10:0]  byte_no = 11'h00;
logic   [10:0]  byte_no_next; 

logic   [31:0]  seq_no;
logic   [31:0]  seq_no_next;


// State
always @ (posedge clk) begin
	state <= state_next;
	byte_no <= byte_no_next;
	
	if (~run | ~have_ip) seq_no <= 32'h0; else seq_no <= seq_no_next;
end


// FSM Combinational
always @* begin

  state_next = state;
  seq_no_next = seq_no;
  byte_no_next = byte_no;
  
  udp_tx_request = 1'b0;
  rx_request = 1'b0;
  
  udp_tx_data = 8'd42;  //answer to...
  
   
	case (state)
		INIT: begin
			
			byte_no_next = 11'h406;
			if ((rx_length  > 10'd171) & have_ip & run) begin state_next = START; end 
		end
		
		START: begin
			udp_tx_request = 1'b1;
			
			if (udp_tx_enable) begin  
				byte_no_next = byte_no - 11'd1; 
				rx_request = 1'b1; 
				state_next = SEQ_NR; 
			end  
		end
		
		SEQ_NR: begin
			byte_no_next = byte_no - 11'd1;
			case (byte_no[2:0])
				3'h5: udp_tx_data = seq_no[31:24];
				3'h4: udp_tx_data = seq_no[23:16];
				3'h3: udp_tx_data = seq_no[15:8];
				3'h2: begin
						udp_tx_data = seq_no[7:0];
						seq_no_next = seq_no + 'h1;
						state_next = I_RXDATA2;
					end
				default: udp_tx_data = byte_no[7:0];
			endcase
        end
		
		I_RXDATA2: begin		
			byte_no_next = byte_no - 11'd1;
			udp_tx_data = rx_data[47:40];
		
			state_next = I_RXDATA1;
		end      

		I_RXDATA1: begin
			byte_no_next = byte_no - 11'd1;
			udp_tx_data = rx_data[39:32];

			state_next = I_RXDATA0;        
		end  

		I_RXDATA0: begin
		  byte_no_next = byte_no - 11'd1;
		  udp_tx_data = rx_data[31:24];

		  state_next = Q_RXDATA2;
		end 		
		
		Q_RXDATA2: begin
			byte_no_next = byte_no - 11'd1;
			udp_tx_data = rx_data[23:16];
		
			state_next = Q_RXDATA1;
		end      

		Q_RXDATA1: begin
			byte_no_next = byte_no - 11'd1;
			udp_tx_data = rx_data[15:8];

			state_next = Q_RXDATA0;        
		end  

		Q_RXDATA0: begin
		  byte_no_next = byte_no - 11'd1;
		  udp_tx_data = rx_data[7:0];
		  
		  if (|byte_no) begin  rx_request = 1'b1; state_next = I_RXDATA2; end else begin state_next = INIT; end
		end 		
	
		default: state_next = INIT;

	endcase // state
  
end // always @*

endmodule