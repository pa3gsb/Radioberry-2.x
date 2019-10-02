/*
  HPSDR - High Performance Software Defined Radio

  Hermes code. 

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 Polyphase decimating filter

 Based on firX8R8 by James Ahlstrom, N2ADR,  (C) 2011
 Modified for use with HPSDR and DC spur removed by Phil Harman, VK6(A)PH, (C) 2013, 2016


 This is a decimate by 2 dual bank Polyphase FIR filter. 

 The coefficients are initially generated for a basic CIC compensating FIR designed for  256 TAPs, 0.01dB ripple, 110dB ultimate 
 rejection FIR. The FIR compensates for the passband droop of the preceding CIC filter. The coefficients were 
 calculated using Matlab (see filter_2.m).
 
 The Matlab coefficients are fractional so are converted to 18 bit integers by scalling the largest coefficient
 to  2^17 - 1. 
 
 Note that when designing the basic FIR that it decimates by 2 hence the output bandwith will be 1/4 the design
 sampling rate and the output will be 50% of the input levels. 
 
 The coefficients are then split into two files, one containing the even  coefficients (0 - 254) and the other
 the odd (1 - 255).  Conventionally these would then be used as the coeffients for the two FIRs that comprise the polyhase FIR.
 
 However, doing so would restrict the maximum sampling rate at the FIR output to 768ksps.  In order to double this, each set of
 coefficients is again split into two files. The even coefficients are then (0 - 126) and (128 - 254) and the odd (1 - 127) and 
 (129 - 255).  

 The coefficents are then converted to *.mif files so they can be loaded into a ROM. The files are 
 
 (0 - 126) 		= coefFa.mif
 (128 - 254) 	= coefFb.mif
 (1 - 127)     = coefEa.mif
 (129 - 255)	= coefEb.mif

 The generation of the mif files is done via an Excel spreadsheet (see Coeffficients.xlsx) 
 
 Hence each ROM needs to hold 18 x 64 coefficients.  However, due to the need to pipline the ROM addresses in practice a 
 18 x 128 ROM is used with the values > 64 equal to zero.  
 
 Note that the mif files are passed to each  ROM as a parameter.  This not supported by the Quartus ROM Megafunction the output
 of which needed to be  edited (against Altera's recommendations!) in order to be used.

 The speed increase is obtained by operating the Ea/Fa and Eb/Fb FIRs in paralled.  Hence a new input sample is fed to both a and b FIRs. 
 
 In order that the input data is multipled by the coefficients in the correct sequence the sample fed to Eb and Fb is sent
 to a RAM address that is equal to the Fa and Ea address plus the mumber of taps.  
 
 A (24+24) x 128 RAM is required due to the intial address offset of the Eb and Fb FIRs. 

   
   Each FIR works as follows:
   
    RAM address     0     1     2     3     4     5     6     7   etc
						-----------------------------------------------
						|     |     |     |     |     |     |     |     
						|	   |     |     |     |     |     |     |     
						------------------------------------------------
				
    Coeff address    0     1     2     3     4     5     6     7   etc
						-----------------------------------------------
						|     |     |     |     |     |     |     |     
						| h0  |  h1 | h2  | h3  | h4  | h5  | h6  | h7    
						------------------------------------------------	
				  
				  
    At reset the ROM and RAM addresses are set to 0 (64 in the case of the Eb/Fb RAM bank)
    The first sample is written to RAM address 0 (64). This cause the sample to be multiplied
    by h0 and accumulated. The RAM address is then decremented and the ROM address incremented.
    The sample in the new RAM address is multiplied by the coefficient from the new ROM address.  
    This process is repeated TAPS times. The code then waits for the next sample to arrive which 
    is written to RAM address + 1. This sample is then multiplied by h0 etc and the process is
    continued TAPS times.		
*/


module recv2_firX2R2 (
   input reset,	
	input clock,
	input x_avail,									// new sample is available
	input signed [INBITS-1:0] x_real,		// x is the sample input
	input signed [INBITS-1:0] x_imag,
	output y_avail,								// new output is available
	output wire signed [OBITS-1:0] y_real,	// y is the filtered output
	output wire signed [OBITS-1:0] y_imag);
	
	localparam ADDRBITS	= 7;					// Address bits for 48 x 128 RAM blocks
   localparam INBITS    = 24;					// width of I and Q input samples	
	
	parameter
		TAPS 			= 64,						   // Number of coefficients per FIR - total is 64 * 4 = 256
   	ABITS			= 24,							// adder bits
		OBITS			= 24;							// output bits
	
	reg [4:0] wstate;								// state machine for write samples
	
	reg  [ADDRBITS-1:0] waddr, raddr;		// write sample memory address
	wire weA, weB, weC, weD;
	reg  signed [ABITS-1:0] Racc, Iacc;
	wire signed [ABITS-1:0] RaccAa, RaccBa, RaccAb, RaccBb;
	wire signed [ABITS-1:0] IaccAa, IaccBa, IaccAb, IaccBb;	
	
// Output is the result of adding 2 by 24 bit results so Racc and Iacc need to be 
// 24 + log2(2) = 24 + 1 = 25 bits wide to prevent DC spur.
// However, since we decimate by 2 the output will be 1/2 the input. Hence we 
// use 24 bits for the Accumulators. 

	assign y_real = Racc[ABITS-1:0];  
	assign y_imag = Iacc[ABITS-1:0];
	
	initial
	begin
		wstate = 0;
		waddr = 0;
		raddr = 0;
	end
	
	always @(posedge clock)
	begin
		if (reset) 
			begin 
				wstate <= 0;
				waddr <= 0;
				raddr <= 0;
				Racc <= 0;
				Iacc <= 0;		
			end 
		else
		begin 
			if (wstate == 2) wstate <= wstate + 1'd1;	// used to set y_avail
			if (wstate == 3) begin
				wstate <= 0;									// reset state machine and increment RAM write address
				waddr <= waddr + 1'd1;
				raddr <= waddr;
			end
			if (x_avail)
			begin
				wstate <= wstate + 1'd1;
				case (wstate)
					0:	begin											// wait for the first x input
							Racc <= RaccAa + RaccAb;			// add accumulators from 'a' and 'b' FIRs
							Iacc <= IaccAa + IaccAb;
						end
					1:	begin											// wait for the next x input
							Racc <= Racc + RaccBa + RaccBb;		
							Iacc <= Iacc + IaccBa + IaccBb;
						end
				endcase
			end
		end
	end
	
	
	// Enable each FIR in sequence
   assign weA 		= (x_avail && wstate == 0);
	assign weB 		= (x_avail && wstate == 1);

	
	// at end of sequence indicate new data is available
	assign y_avail = (wstate == 2);
	
	// Dual bank polyphase decimate by 2 FIR. Note that second bank needs a RAM write offset of TAPS.

	recv2_fir256d #("coefEa.mif", ABITS, TAPS) A (reset, clock, waddr, raddr, weA, x_real, x_imag, RaccAa, IaccAa);				// first bank odd coeff
	recv2_fir256d #("coefEb.mif", ABITS, TAPS) B (reset, clock, (waddr + 7'd64), raddr, weA, x_real, x_imag, RaccAb, IaccAb);	// second bank 
	recv2_fir256d #("coefFa.mif", ABITS, TAPS) C (reset, clock, waddr, raddr, weB, x_real, x_imag, RaccBa, IaccBa);  				// first bank even coeff
	recv2_fir256d #("coefFb.mif", ABITS, TAPS) D (reset, clock, (waddr + 7'd64), raddr, weB, x_real, x_imag, RaccBb, IaccBb); 	// second bank  


endmodule


// This filter waits until a new sample is written to memory at waddr.  Then
// it starts by multiplying that sample by coef[0], the next prior sample
// by coef[1], (etc.) and accumulating.  For R=2 decimation, coef[1] is the
// coeficient prior to coef[0].
// When reading from the RAM and ROM we need to allow 3 clock pulses from presenting the 
// read address until the data is available. 

module recv2_fir256d(

	input reset,
	input clock,
	input [ADDRBITS-1:0] waddr,							// memory write address
	input [ADDRBITS-1:0] raddr,							// memory read address
	input we,													// memory write enable
	input signed [INBITS-1:0] x_real,					// sample to write
	input signed [INBITS-1:0] x_imag,
	output reg signed [ABITS-1:0] Raccum,
	output reg signed [ABITS-1:0] Iaccum
	);

	localparam ADDRBITS	= 7;								// Address bits for 18/36 x 128 ROM/RAM
	localparam COEFBITS	= 18;								// coefficient bits
	localparam INBITS		= 24;								// I and Q sample width 
	
	parameter MifFile	= "xx.mif";							// ROM coefficients
	parameter ABITS	= 0;									// adder bits
	parameter TAPS		= 0;									// number of filter taps

	reg [ADDRBITS-1:0] caddr;								// read address for  coef
	wire [INBITS*2-1:0] q;									// I/Q sample read from memory
	reg  [INBITS*2-1:0] reg_q;
	wire signed [INBITS-1:0] q_real, q_imag;			// I/Q sample read from memory
	wire signed [COEFBITS-1:0] coef; 	      		// coefficient read from memory
	reg signed  [COEFBITS-1:0] reg_coef; 				// coefficient read from memory
	reg signed  [41:0] Rmult, Imult;						// multiplier result   24 * 18 bits = 42 bits 
	reg signed  [41:0] RmultSum, ImultSum;				// multiplier result
	reg [ADDRBITS:0] counter;								// count TAPS samples, size to allow for pipeline latency
   reg [ADDRBITS-1:0] read_address;
	
	
	assign q_real = reg_q[INBITS*2-1:INBITS];
	assign q_imag = reg_q[INBITS-1:0];

	recv2_firromH #(MifFile) roma (caddr, clock, coef);		// coefficient ROM 18 X 128
	recv2_firram48 rama (clock, {x_real, x_imag}, read_address, waddr, we, q);  	// sample RAM 48 X 128;  48 bit = 24 bits I and 24 bits Q

	
	always @(posedge clock)
	begin
	   if(reset)
			begin 
				Rmult <= 0;
				Imult <= 0;
				Raccum <= 0;
				Iaccum <= 0;
			end 
		else if (we)		// Wait until a new sample is written to memory
			begin
				counter <= TAPS[ADDRBITS:0] + 7'd3;			// count samples and pipeline latency (delay of 3 clocks from address being presented)
				read_address <= raddr;						// RAM read address -> newest sample
				caddr <= 1'd0;									// start at coefficient zero
				Raccum <= 0;
				Iaccum <= 0;
			end
		else
			begin		// main pipeline here
				if (counter < (TAPS[ADDRBITS:0]) + 7'd1)
				begin
					Rmult <= q_real * reg_coef;
					Imult <= q_imag * reg_coef;
					Raccum <= Raccum + Rmult[39:16] + Rmult[15];  // Truncate 42 bits down to 24 bits to prevent DC spur.
					Iaccum <= Iaccum + Imult[39:16] + Imult[15];  // Multiply by 4 to match gain of previous code.
				end
				if (counter > 0)
				begin
					counter <= counter - 1'd1;
					read_address <= read_address - 1'd1;	// move to prior sample
					caddr <= caddr + 1'd1;						// move to next coefficient
					reg_q <= q;
					reg_coef <= coef;
				end
			end
	end
endmodule
