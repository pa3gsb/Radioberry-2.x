//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          Transmitter code 
//------------------------------------------------------------------------------------------------------------------------------------------------------------

module transmitter(
	reset,
	clk,                  
	frequency,
	afTxFIFO, 
	afTxFIFOEmpty, 
	afTxFIFOReadStrobe,
	CW_RF,
	out_data,
	PTT,
	CW_PTT,
	LED);

input wire reset;
input wire clk;
input [31:0] frequency;
input  wire [31:0]afTxFIFO; 	
input wire afTxFIFOEmpty;
output wire afTxFIFOReadStrobe;
input wire [15:0] CW_RF; 
output reg [13:0] out_data;
input wire PTT;
input wire CW_PTT;
output wire LED;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          Read IQ data from txFIFO
//------------------------------------------------------------------------------------------------------------------------------------------------------------

wire pulse;
pulsegen pulse_inst (.sig(req1), .rst(reset), .clk(clk), .pulse(pulse)); 

assign afTxFIFOReadStrobe = !afTxFIFOEmpty ? pulse : 1'b0;

reg  [31:0] tx_IQ_data;
always @(posedge clk)
begin
	if (reset)
		tx_IQ_data <= 32'b0;

	if (afTxFIFOReadStrobe)
		tx_IQ_data <= afTxFIFO;
	
end	

reg signed [15:0]fir_i;
reg signed [15:0]fir_q;

// latch I&Q data on strobe from FIR
always @ (posedge clk)
begin 
	if (req1) begin 
		fir_i = tx_IQ_data[31:16];
		fir_q = tx_IQ_data[15:0];
	end 
end


// Interpolate I/Q samples from 48 kHz to the clock frequency
wire req1, req2;
wire [19:0] y1_r, y1_i; 
wire [15:0] y2_r, y2_i;

FirInterp8_1024 fi (clk, req2, req1, fir_i, fir_q, y1_r, y1_i);  // req2 enables an output sample, req1 requests next input sample.

// GBITS reduced to 31
CicInterpM5 #(.RRRR(192), .IBITS(20), .OBITS(16), .GBITS(31)) in2 ( clk, 1'd1, req2, y1_r, y1_i, y2_r, y2_i);

//---------------------------------------------------------
//    CORDIC NCO 
//---------------------------------------------------------
wire signed [14:0] cordic_i_out;

cpl_cordic #(.OUT_WIDTH(16))
 		cordic_inst (.clock(clk), .frequency(frequency), .in_data_I(CW_PTT? CW_RF: y2_i),			
		.in_data_Q(CW_PTT? 16'd0: y2_r), .out_data_I(cordic_i_out), .out_data_Q());		


wire signed [14:0] gated;

assign gated = PTT ? cordic_i_out : 14'd0;
always @ (negedge clk) out_data[13:0] <= gated[13:0]; 

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                        Heartbeat (LED flashes twice as fast when PTT active)
//------------------------------------------------------------------------------------------------------------------------------------------------------------
reg[26:0]counter;
always @(posedge clk) counter = counter + 1'b1;
assign LED = PTT ? counter[24] : counter[26];  
 
endmodule