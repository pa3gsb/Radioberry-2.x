//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          Transmitter code 
//------------------------------------------------------------------------------------------------------------------------------------------------------------

module transmitter(
	reset,
	clk,                  
	frequency,
	tsiq_data, 
	tsiq_read_strobe,
	tsiq_valid,
	CW_RF,
	out_data,
	PTT,
	CW_PTT,
	LED);

input wire reset;
input wire clk;
input [31:0] frequency;
input  wire [31:0]tsiq_data; 	
output wire tsiq_read_strobe;
input wire tsiq_valid;
input wire [15:0] CW_RF; 
output reg [11:0] out_data;
input wire PTT;
input wire CW_PTT;
output wire LED;

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                          Read IQ data from txFIFO
//------------------------------------------------------------------------------------------------------------------------------------------------------------
reg signed [15:0]fir_i;
reg signed [15:0]fir_q;

assign tsiq_read_strobe = req1;

// latch I&Q data on strobe from FIR
always @ (posedge clk)
begin 
	if (req1 & tsiq_valid ) begin 
		fir_i <= tsiq_data[31:16];
		fir_q <= tsiq_data[15:0];
	end 
end

// Interpolate I/Q samples from 48 kHz to the clock frequency
wire req1, req2;
wire [19:0] y1_r, y1_i; 
wire [15:0] y2_r, y2_i;

FirInterp8_1024 fi (clk, req2, req1, fir_i, fir_q, y1_r, y1_i);  // req2 enables an output sample, req1 requests next input sample.

// GBITS reduced to 31
CicInterpM5 #(.RRRR(200), .IBITS(20), .OBITS(16), .GBITS(31)) in2 ( clk, 1'd1, req2, y1_r, y1_i, y2_r, y2_i);

//---------------------------------------------------------
//    CORDIC NCO 
//---------------------------------------------------------
wire signed [15:0] cordic_i_out;

cpl_cordic #(.OUT_WIDTH(16))
 		cordic_inst (.clock(clk), .frequency(frequency), .in_data_I(CW_PTT? CW_RF: y2_i),			
		.in_data_Q(CW_PTT? 16'd0: y2_r), .out_data_I(cordic_i_out), .out_data_Q());		


wire signed [15:0] gated;

assign gated = PTT ? (cordic_i_out >>> 4) : 16'd0;
always @ (negedge clk) out_data <= gated[11:0]; 

//------------------------------------------------------------------------------------------------------------------------------------------------------------
//                        Heartbeat (LED flashes twice as fast when PTT active)
//------------------------------------------------------------------------------------------------------------------------------------------------------------
reg[26:0]counter;
always @(posedge clk) counter = counter + 1'b1;
assign LED = PTT ? counter[24] : counter[26];  
 
endmodule