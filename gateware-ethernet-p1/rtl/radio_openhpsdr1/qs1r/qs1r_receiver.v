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
//           Copyright (c) 2008 Alex Shovkoplyas, VE3NEA
//------------------------------------------------------------------------------


//If you make any changes to this file, please leave a comment here





module qs1r_receiver(
  input clock,                  //122.88 MHz
  input [1:0] rate,             //00=48, 01=96, 10=192 kHz
  input [31:0] frequency,
  output out_strobe,

  input signed [15:0] in_data,

  output [23:0] out_data_I,
  output [23:0] out_data_Q
  );


  
  
  
  
//------------------------------------------------------------------------------
//                               cordic
//------------------------------------------------------------------------------
qs1r_cordic cordic_inst(
  .clock(clock),
  .in_data(in_data),             //16 bit 
  .frequency(frequency),         //32 bit
  .out_data_I(cordic_outdata_I), //22 bit
  .out_data_Q(cordic_outdata_Q)
  );
 

wire signed [21:0] cordic_outdata_I;
wire signed [21:0] cordic_outdata_Q;
  

  
  
  
  
//------------------------------------------------------------------------------
//         register-based CIC decimator #1, decimation factor 32/64/128
//------------------------------------------------------------------------------
//I channel
qs1r_varcic #(.STAGES(4), .DECIMATION(20), .IN_WIDTH(22), .ACC_WIDTH(50), .OUT_WIDTH(24))
  varcic_inst_I1(
    .clock(clock),
    .in_strobe(1'b1),
    .extra_decimation(rate),
    .out_strobe(cic_outstrobe_1),
    .in_data(cordic_outdata_I),
    .out_data(cic_outdata_I1)
    );


//Q channel
qs1r_varcic #(.STAGES(4), .DECIMATION(20), .IN_WIDTH(22), .ACC_WIDTH(50), .OUT_WIDTH(24))
  varcic_inst_Q1(
    .clock(clock),
    .in_strobe(1'b1),
    .extra_decimation(rate),
    .out_strobe(),
    .in_data(cordic_outdata_Q),
    .out_data(cic_outdata_Q1)
    );


wire cic_outstrobe_1;
wire signed [23:0] cic_outdata_I1;
wire signed [23:0] cic_outdata_Q1;






//------------------------------------------------------------------------------
//            memory-based CIC decimator #2, decimation factor 10
//------------------------------------------------------------------------------
//I channel
qs1r_memcic #(.STAGES(13), .DECIMATION(10), .ACC_WIDTH(68)) 
  memcic_inst_I(
    .clock(clock),
    .in_strobe(cic_outstrobe_1),
    .out_strobe(cic_outstrobe_2),
    .in_data(cic_outdata_I1),
    .out_data(cic_outdata_I2)
    );
	 

//Q channel
qs1r_memcic #(.STAGES(13), .DECIMATION(10), .ACC_WIDTH(68)) 
  memcic_inst_Q(
    .clock(clock),
    .in_strobe(cic_outstrobe_1),
    .out_strobe(),
    .in_data(cic_outdata_Q1),
    .out_data(cic_outdata_Q2)
    );
	 
	 
wire cic_outstrobe_2;
wire signed [23:0] cic_outdata_I2;
wire signed [23:0] cic_outdata_Q2;
	 

	 
	 
	 
	 
//------------------------------------------------------------------------------
//                     FIR coefficients and sequencing
//------------------------------------------------------------------------------
wire signed [23:0] fir_coeff;

qs1r_fir_coeffs fir_coeffs_inst(
  .clock(clock),
  .start(cic_outstrobe_2),
  .coeff(fir_coeff)
  );


  
  
  
  
//------------------------------------------------------------------------------
//                            FIR decimator
//------------------------------------------------------------------------------
qs1r_fir #(.OUT_WIDTH(24))
  fir_inst_I(
    .clock(clock),
    .start(cic_outstrobe_2), 
    .coeff(fir_coeff),
    .in_data(cic_outdata_I2),
    .out_data(out_data_I),
    .out_strobe(out_strobe)
    );


qs1r_fir #(.OUT_WIDTH(24))
  fir_inst_Q(
    .clock(clock),
    .start(cic_outstrobe_2),
    .coeff(fir_coeff),
    .in_data(cic_outdata_Q2),
    .out_data(out_data_Q),
    .out_strobe()
    );



endmodule
