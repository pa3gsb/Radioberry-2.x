/*
--------------------------------------------------------------------------------
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
Boston, MA  02110-1301, USA.
--------------------------------------------------------------------------------
*/


//------------------------------------------------------------------------------
//           Copyright (c) 2016 James C. Ahlstrom, N2ADR
//------------------------------------------------------------------------------

// 2016 Nov 26 - new VNA logic James Ahlstrom N2ADR

// This module scans a set of vna_count frequencies starting from Tx_frequency_in and adding freq_delta for each point.  Each
// point is the average of 1024 cordic I/Q outputs.  For each point, the frequency is changed, then there is a pause to allow things
// to stabilize, then the average is taken and returned with the output_strobe.  The PC receives the points as normal I/Q samples
// at a rate of 8000 sps.  A zero sample is output at the start of the scan and the next sample is for the first frequency.

// This module also sets the Tx frequency for the original VNA method in which the PC scans the frequencies.

module vna_scanner(
  input clock,
  input [31:0] freq_delta,
  output reg output_strobe,
  input signed [17:0] cordic_data_I,
  input signed [17:0] cordic_data_Q,
  output reg signed [23:0] out_data_I,
  output reg signed [23:0] out_data_Q,
  // VNA modes are PC-scan and FPGA-scan
  input vna,	// True for either scanning by the FPGA or PC
  input [31:0] Tx_frequency_in,
  output reg [31:0] Tx_frequency_out,
  input [15:0] vna_count
  );

  parameter CICRATE;
  parameter RATE48;		// The decimation for 48000 sps

reg [2:0] vna_state;		// state machine for both VNA modes
reg [13:0] vna_decimation;	// count up DECIMATION clocks, and then output a sample; increase bits for clock > 131 MHz
reg [15:0] vna_counter;		// count the number of scan points until we get to vna_count desired points
reg [9:0] data_counter;		// Add up 1024 cordic samples per output sample ; 2**10 = 1024
reg signed [27:0] vna_I, vna_Q;	// accumulator for I/Q cordic samples: 18 bit cordic * 10-bits = 28 bits

localparam DECIMATION = (RATE48 * CICRATE * 8) * 6;	// The decimation; the number of clocks per output sample

localparam VNA_STARTUP		= 0;	// States in the state machine
localparam VNA_PC_SCAN		= 1;
localparam VNA_TAKE_DATA	= 2;
localparam VNA_ZERO_DATA	= 3;
localparam VNA_RETURN_DATUM1	= 4;
localparam VNA_RETURN_DATUM2	= 5;
localparam VNA_CHANGE_FREQ	= 6;
localparam VNA_WAIT_STABILIZE	= 7;


always @(posedge clock)		// state machine for VNA
begin
	if ( ! vna)
	begin	// Not in VNA mode; operate as a regular receiver
		Tx_frequency_out <= Tx_frequency_in;
		vna_state <= VNA_STARTUP;
	end
	else case (vna_state)
	VNA_STARTUP:		// Start VNA mode; zero the Rx and Tx frequencies to synchronize the cordics to zero phase
	begin
		Tx_frequency_out <= 1'b0;
		vna_counter <= 1'd1;
		vna_decimation <= DECIMATION;
		output_strobe <= 1'b0;
		if (vna_count == 1'd0)
			vna_state <= VNA_PC_SCAN;
		else
			vna_state <= VNA_CHANGE_FREQ;
	end
	VNA_PC_SCAN:		// stay in this VNA state when the PC scans the VNA points
	begin
		Tx_frequency_out <= Tx_frequency_in;
		if (vna_count != 1'd0)		// change to vna_count
			vna_state <= VNA_STARTUP;
	end
	VNA_TAKE_DATA:		// add up points to produce a sample
	begin
		vna_decimation <= vna_decimation - 1'd1;
		vna_I <= vna_I + cordic_data_I;
		vna_Q <= vna_Q + cordic_data_Q;
		if (data_counter == 1'b0)
			vna_state <= VNA_RETURN_DATUM1;
		else
			data_counter <= data_counter - 1'd1;
	end
	VNA_ZERO_DATA:		// make a zero sample
	begin
		vna_decimation <= vna_decimation - 1'd1;
		if (data_counter == 1'b0)
			vna_state <= VNA_RETURN_DATUM1;
		else
			data_counter <= data_counter - 1'd1;
	end
	VNA_RETURN_DATUM1:		// Return the sample
	begin
		vna_decimation <= vna_decimation - 1'd1;
		out_data_I <= vna_I[27:4];
		out_data_Q <= vna_Q[27:4];
		vna_state <= VNA_RETURN_DATUM2;
	end
	VNA_RETURN_DATUM2:		// Return the sample
	begin
		vna_decimation <= vna_decimation - 1'd1;
		if (vna_count == 1'd0)		// change to vna_count
			vna_state <= VNA_STARTUP;
		else
		begin
			output_strobe <= 1'b1;
			vna_state <= VNA_CHANGE_FREQ;
		end
	end
	VNA_CHANGE_FREQ:		// done with samples; change frequency
	begin
		vna_decimation <= vna_decimation - 1'd1;
		output_strobe <= 1'b0;
		if (vna_counter == 1'd1)
		begin
			Tx_frequency_out <= Tx_frequency_in;	// starting frequency for scan
			vna_counter <= 1'd0;
		end
		else if (vna_counter == 1'd0)
			vna_counter <= vna_count;
		else
		begin
			vna_counter <= vna_counter - 1'd1;
			Tx_frequency_out <= Tx_frequency_out + freq_delta;	// freq_delta is the frequency to add for each point
		end
		vna_state <= VNA_WAIT_STABILIZE;
	end
	VNA_WAIT_STABILIZE:		// Output samples at 8000 sps.  Allow time for output to stabilize after a frequency change.
	begin
		if (vna_decimation == 1'b0)
		begin
			vna_I <= 1'd0;
			vna_Q <= 1'd0;
			vna_decimation <= DECIMATION - 1;
			data_counter <= 10'd1023;
			if (vna_counter == 0)
				vna_state <= VNA_ZERO_DATA;
			else
				vna_state <= VNA_TAKE_DATA;
		end
		else
			vna_decimation <= vna_decimation - 1'd1;
	end
	endcase
end
endmodule
