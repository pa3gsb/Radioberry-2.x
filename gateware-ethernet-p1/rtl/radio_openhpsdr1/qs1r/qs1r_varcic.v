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



module qs1r_varcic( extra_decimation, clock, in_strobe,  out_strobe, in_data, out_data );


  //design parameters
  parameter STAGES = 5;
  parameter DECIMATION = 320;  
  parameter IN_WIDTH = 22;


  //computed parameters
  //ACC_WIDTH = IN_WIDTH + Ceil(STAGES * Log2(decimation factor))
  //OUT_WIDTH = IN_WIDTH + Ceil(Log2(decimation factor) / 2)
  parameter ACC_WIDTH = 64;
  parameter OUT_WIDTH = 27;

  //00 = DECIMATION*4, 01 = DECIMATION*2, 10 = DECIMATION
  input [1:0] extra_decimation;
  
  input clock;
  input in_strobe;
  output reg out_strobe;

  input signed [IN_WIDTH-1:0] in_data;
  output reg signed [OUT_WIDTH-1:0] out_data;





//------------------------------------------------------------------------------
//                               control
//------------------------------------------------------------------------------
reg [15:0] sample_no;
initial sample_no = 16'd0;


always @(posedge clock)
  if (in_strobe)
    begin
    if (sample_no == ((DECIMATION << (2-extra_decimation))-1))
      begin
      sample_no <= 0;
      out_strobe <= 1;
      end
    else
      begin
      sample_no <= sample_no + 8'd1;
      out_strobe <= 0;
      end
    end

  else
    out_strobe <= 0;






//------------------------------------------------------------------------------
//                                stages
//------------------------------------------------------------------------------
wire signed [ACC_WIDTH-1:0] integrator_data [0:STAGES];
wire signed [ACC_WIDTH-1:0] comb_data [0:STAGES];


assign integrator_data[0] = in_data;
assign comb_data[0] = integrator_data[STAGES];


genvar i;
generate
  for (i=0; i<STAGES; i=i+1)
    begin : cic_stages

    qs1r_cic_integrator #(ACC_WIDTH) cic_integrator_inst(
      .clock(clock),
      .strobe(in_strobe),
      .in_data(integrator_data[i]),
      .out_data(integrator_data[i+1])
      );


    qs1r_cic_comb #(ACC_WIDTH) cic_comb_inst(
      .clock(clock),
      .strobe(out_strobe),
      .in_data(comb_data[i]),
      .out_data(comb_data[i+1])
      );
    end
endgenerate







//------------------------------------------------------------------------------
//                            output rounding
//------------------------------------------------------------------------------
localparam MSB0 = ACC_WIDTH - 1;            //63
localparam LSB0 = ACC_WIDTH - OUT_WIDTH;    //41

localparam MSB1 = MSB0 - STAGES;            //58
localparam LSB1 = LSB0 - STAGES;            //36

localparam MSB2 = MSB1 - STAGES;            //53
localparam LSB2 = LSB1 - STAGES;            //31


always @(posedge clock)
  case (extra_decimation)
    0: out_data <= comb_data[STAGES][MSB0:LSB0] + comb_data[STAGES][LSB0-1];
    1: out_data <= comb_data[STAGES][MSB1:LSB1] + comb_data[STAGES][LSB1-1];
    2: out_data <= comb_data[STAGES][MSB2:LSB2] + comb_data[STAGES][LSB2-1];
  endcase



endmodule

