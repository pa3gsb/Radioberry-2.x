//
//  HPSDR - High Performance Software Defined Radio
//
//  Hermes code. 
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// Polyphase decimating filter

// Based on firX8R8 by James Ahlstrom, N2ADR,  (C) 2011
// Modified for use with HPSDR and DC spur removed by Phil Harman, VK6APH, (C) 2013


// This is a decimate by 8 Polyphase FIR filter. Since it decimates by 8 the output signal
// level will be 1/8 the input level.  The filter coeficients are distributed between the 8 
// FIR filters such that the first FIR receives coeficients 0, 7, 15... the second 1, 8, 16.. the 
// third 2, 9, 17.. etc.  The coeficients are calculated as per normal but there is no need to 
// compensate for the sinx/x shape of the preceeding CIC filters. This is because the filter 
// decimates by 8 and the droop of the CIC at 1/8th its fs/2 is neglibible. 
// The filter coefficients are in the file "coefL8.txt". This is split into 8 individual 
// Quartus ROM *.mif files.

// The filter coefficients are also attenuated such that the result of the multiply and accumalate 
// does not exceed 24 bits. 

// Note: Gain is higher than previous filter code by 6dB so reduce outside this module.
// FIR filters
//
// ROM init file:		REQUIRED, with 256 or 512 coefficients.  See below.
// Number of taps:	NTAPS.
// Input bits:			18 fixed.
// Output bits:		OBITS, default 24.
// Adder bits:			ABITS, default 24.

// This requires eight MifFile's.
// Maximum NTAPS is 8 * (previous and current decimation) less overhead.
// Maximum NTAPS is 2048 (or less).

module firX8R8 (	
	input clock,
	input x_avail,									// new sample is available
	input signed [MBITS-1:0] x_real,			// x is the sample input
	input signed [MBITS-1:0] x_imag,
	output reg y_avail,							// new output is available
	output wire signed [OBITS-1:0] y_real,	// y is the filtered output
	output wire signed [OBITS-1:0] y_imag);
	
	localparam ADDRBITS	= 8;					// Address bits for 18/36 X 256 rom/ram blocks
	localparam MBITS	= 18;						// multiplier bits == input bits	
	
	parameter
		TAPS 			= NTAPS / 8,				// Must be even by 8
   		ABITS			= 24,							// adder bits
		OBITS			= 24,							// output bits
		NTAPS			= 976;						// number of filter taps, even by 8	
	
	reg [4:0] wstate;								// state machine for write samples
	
	reg  [ADDRBITS-1:0] waddr;					// write sample memory address
	reg weA, weB, weC, weD, weE, weF, weG, weH;
	reg  signed [ABITS-1:0] Racc, Iacc;
	wire signed [ABITS-1:0] RaccA, RaccB, RaccC, RaccD, RaccE, RaccF, RaccG, RaccH;
	wire signed [ABITS-1:0] IaccA, IaccB, IaccC, IaccD, IaccE, IaccF, IaccG, IaccH;	
	
// Output is the result of adding 8 by 24 bit results so Racc and Iacc need to be 
// 24 + log2(8) = 24 + 3 = 27 bits wide to prevent DC spur.
// However, since we decimate by 8 the output will be 1/8 the input. Hence we 
// use 24 bits for the Accumulators. 

	assign y_real = Racc[ABITS-1:0];  
	assign y_imag = Iacc[ABITS-1:0];
	
	initial
	begin
		wstate = 0;
		waddr = 0;
	end
	
	always @(posedge clock)
	begin
		if (wstate == 8) wstate <= wstate + 1'd1;	// used to set y_avail
		if (wstate == 9) begin
			wstate <= 0;									// reset state machine and increment RAM write address
			waddr <= waddr + 1'd1;
		end
		if (x_avail)
		begin
			wstate <= wstate + 1'd1;
			case (wstate)
				0:	begin										// wait for the first x input
						Racc <= RaccA;						// add accumulators
						Iacc <= IaccA;
					end
				1:	begin										// wait for the next x input
						Racc <= Racc + RaccB;		
						Iacc <= Iacc + IaccB;
					end
				2:	begin	
						Racc <= Racc + RaccC;		
						Iacc <= Iacc + IaccC;
					end
				3:	begin
						Racc <= Racc + RaccD;		
						Iacc <= Iacc + IaccD;
					end
				4:	begin
						Racc <= Racc + RaccE;		
						Iacc <= Iacc + IaccE;
					end
				5:	begin
						Racc <= Racc + RaccF;		
						Iacc <= Iacc + IaccF;
					end
				6:	begin
						Racc <= Racc + RaccG;		
						Iacc <= Iacc + IaccG;
					end
				7: begin										// wait for the last x input
						Racc <= Racc + RaccH;
						Iacc <= Iacc + IaccH;
					end
			endcase
		end
	end
	
	
	// Enable each FIR in sequence
	assign weA 		= (x_avail && wstate == 0);
	assign weB 		= (x_avail && wstate == 1);
	assign weC 		= (x_avail && wstate == 2);
	assign weD 		= (x_avail && wstate == 3);
	assign weE 		= (x_avail && wstate == 4);
	assign weF 		= (x_avail && wstate == 5);
	assign weG 		= (x_avail && wstate == 6);
	assign weH 		= (x_avail && wstate == 7);
	
	// at end of sequence indicate new data is available
	assign y_avail = (wstate == 8);

	fir256 #("coefL8A.mif", ABITS, TAPS) A (clock, waddr, weA, x_real, x_imag, RaccA, IaccA);
	fir256 #("coefL8B.mif", ABITS, TAPS) B (clock, waddr, weB, x_real, x_imag, RaccB, IaccB);
	fir256 #("coefL8C.mif", ABITS, TAPS) C (clock, waddr, weC, x_real, x_imag, RaccC, IaccC);
	fir256 #("coefL8D.mif", ABITS, TAPS) D (clock, waddr, weD, x_real, x_imag, RaccD, IaccD);
	fir256 #("coefL8E.mif", ABITS, TAPS) E (clock, waddr, weE, x_real, x_imag, RaccE, IaccE);
	fir256 #("coefL8F.mif", ABITS, TAPS) F (clock, waddr, weF, x_real, x_imag, RaccF, IaccF);
	fir256 #("coefL8G.mif", ABITS, TAPS) G (clock, waddr, weG, x_real, x_imag, RaccG, IaccG);
	fir256 #("coefL8H.mif", ABITS, TAPS) H (clock, waddr, weH, x_real, x_imag, RaccH, IaccH);
	
endmodule


// This filter waits until a new sample is written to memory at waddr.  Then
// it starts by multiplying that sample by coef[0], the next prior sample
// by coef[1], (etc.) and accumulating.  For R=8 decimation, coef[1] is the
// coeficient 8 prior to coef[0].
// When reading from the RAM we need to allow 3 clock pulses from presenting the 
// read address until the data is available. 

module fir256(

	input clock,
	input [ADDRBITS-1:0] waddr,							// memory write address
	input we,													// memory write enable
	input signed [MBITS-1:0] x_real,						// sample to write
	input signed [MBITS-1:0] x_imag,
	output signed [ABITS-1:0] Raccum,
	output signed [ABITS-1:0] Iaccum
	);

	localparam ADDRBITS	= 8;								// Address bits for 18/36 X 256 rom/ram blocks
	localparam MBITS		= 18;								// multiplier bits == input bits
	
	parameter MifFile	= "xx.mif";							// ROM coefficients
	parameter ABITS	= 0;									// adder bits
	parameter TAPS		= 0;									// number of filter taps, max 2**ADDRBITS

	reg [ADDRBITS-1:0] raddr, caddr;						// read address for sample and coef
	wire [MBITS*2-1:0] q;									// I/Q sample read from memory
	reg  [MBITS*2-1:0] reg_q;
	wire signed [MBITS-1:0] q_real, q_imag;			// I/Q sample read from memory
	wire signed [MBITS-1:0] coef;							// coefficient read from memory
	reg  signed [MBITS-1:0] reg_coef;
	reg signed [MBITS*2-1:0] Rmult, Imult;				// multiplier result
	reg signed [MBITS*2-1:0] RmultSum, ImultSum;		// multiplier result
	reg [ADDRBITS:0] counter;								// count TAPS samples

	//reg fir_step;										// Pipeline register for fir

	assign q_real = reg_q[MBITS*2-1:MBITS];
	assign q_imag = reg_q[MBITS-1:0];

	firromH #(MifFile) rom (caddr, clock, coef);		// coefficient ROM 18 X 256
	firram36 ram (clock, {x_real, x_imag}, raddr, waddr, we, q);  	// sample RAM 36 X 256;  36 bit == 18 bits I and 18 bits Q
	
	always @(posedge clock)
	begin
		if (we)		// Wait until a new sample is written to memory
			begin
				counter = TAPS[ADDRBITS:0] + 4;			// count samples and pipeline latency (delay of 3 clocks from address being presented)
				raddr = waddr;									// read address -> newest sample
				caddr = 1'd0;									// start at coefficient zero
				Raccum <= 0;
				Iaccum <= 0;
				Rmult <= 0;
				Imult <= 0;
				//fir_step <= 1'b1;
			end
		else
			begin		// main pipeline here
				if (counter < (TAPS[ADDRBITS:0] + 2))
				begin
					//if (fir_step)
					//begin
						Rmult <= q_real * reg_coef;
						Raccum <= Raccum + Rmult[35:12] + Rmult[11];  // truncate 36 bits down to 24 bits to prevent DC spur
						//fir_step <= 1'b0;
					//end
					//else 
					//begin
						Imult <= q_imag * reg_coef;
						Iaccum <= Iaccum + Imult[35:12] + Imult[11];
						//fir_step <= 1'b1;
					//end
				end


				//if (~fir_step & (counter > 0))
				if (counter > 0)
				begin
					counter <= counter - 1'd1;
					raddr <= raddr - 1'd1;						// move to prior sample
					caddr <= caddr + 1'd1;						// move to next coefficient
					reg_q <= q;
					reg_coef <= coef;
				end
			end
	end
endmodule
