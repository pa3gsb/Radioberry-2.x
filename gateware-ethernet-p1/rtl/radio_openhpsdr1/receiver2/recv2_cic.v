//
// cic - A Cascaded Integrator-Comb filter
//
// Copyright (c) 2008 Alex Shovkoplyas, VE3NEA
// Copyright (c) 2013 Phil Harman, VK6PH
// Copyright (c) 2015 Jeremy McDermond, NH6Z
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
// Boston, MA  02110-1301, USA.


// 2013 Jan 26	- Modified to accept decimation values from 1-40. VK6APH 

module recv2_cic(reset, decimation, clock, in_strobe,  out_strobe, in_data, out_data );

  //design parameters
  parameter STAGES = 5; //  Sections of both Comb and Integrate
  parameter MIN_DECIMATION = 2;  // If MIN = MAX, we are single-rate filter
  parameter MAX_DECIMATION = 40;
  parameter IN_WIDTH = 18;
  parameter OUT_WIDTH = 18;

  // derived parameters
  parameter ACC_WIDTH = IN_WIDTH + (STAGES * $clog2(MAX_DECIMATION));
  
  input [$clog2(MAX_DECIMATION):0] decimation; 
  
  input reset;
  input clock;
  input in_strobe;
  output reg out_strobe;

  input signed [IN_WIDTH-1:0] in_data;
  output signed [OUT_WIDTH-1:0] out_data;


//------------------------------------------------------------------------------
//                               control
//------------------------------------------------------------------------------
reg [$clog2(MAX_DECIMATION)-1:0] sample_no = 0;

generate
if(MIN_DECIMATION == MAX_DECIMATION)
	always @(posedge clock)
		if (in_strobe) 
			if (sample_no == (MAX_DECIMATION - 1'd1)) begin
				sample_no <= 0;
				out_strobe <= 1;
			end else begin
				sample_no <= sample_no + 1'd1;
     				out_strobe <= 0;
			end
		else
			out_strobe <= 0;
else
	always @(posedge clock)
		if (in_strobe) 
			if (sample_no == (decimation - 1'd1)) begin
				sample_no <= 0;
				out_strobe <= 1;
			end else begin
				sample_no <= sample_no + 1'd1;
     				out_strobe <= 0;
			end
		else
			out_strobe <= 0;
endgenerate

//------------------------------------------------------------------------------
//                                stages
//------------------------------------------------------------------------------
reg signed [ACC_WIDTH-1:0] integrator_data [1:STAGES] = '{default: '0};
reg signed [ACC_WIDTH-1:0] comb_data [1:STAGES] = '{default: '0};
reg signed [ACC_WIDTH-1:0] comb_last [0:STAGES] = '{default: '0};

always @(posedge clock) begin
	integer index;

	//  Integrators
	if(in_strobe) begin
		integrator_data[1] <= integrator_data[1] + in_data;
		for(index = 1; index < STAGES; index = index + 1) begin
			integrator_data[index + 1] <= integrator_data[index] + integrator_data[index+1];
		end
	end

	// Combs
	if(out_strobe) begin
		comb_data[1] <= integrator_data[STAGES] - comb_last[0];
		comb_last[0] <= integrator_data[STAGES];
		for(index = 1; index < STAGES; index = index + 1) begin
			comb_data[index + 1] <= comb_data[index] - comb_last[index];
			comb_last[index] <= comb_data[index]; 
		end
	end
end

//------------------------------------------------------------------------------
//                            output rounding
//------------------------------------------------------------------------------

genvar i;
generate
	if(MIN_DECIMATION == MAX_DECIMATION) begin
		assign out_data = comb_data[STAGES][ACC_WIDTH - 1 -: OUT_WIDTH] + comb_data[STAGES][ACC_WIDTH - OUT_WIDTH - 1];
	end else begin
		wire [$clog2(ACC_WIDTH)-1:0] msb [MAX_DECIMATION:MIN_DECIMATION];
		for(i = MIN_DECIMATION; i <= MAX_DECIMATION; i = i + 1) begin: round_position
			assign msb[i] = IN_WIDTH + ($clog2(i) * STAGES) - 1 ;
		end

		assign out_data = comb_data[STAGES][msb[decimation] -: OUT_WIDTH] + comb_data[STAGES][msb[decimation] - OUT_WIDTH];
	end
endgenerate

endmodule

