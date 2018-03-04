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

// 2015 Jan 31 - udated for Hermes-Lite 12bit Steve Haynal KF7O

module cic( clock, in_strobe, out_strobe, in_data, out_data );

//design parameters
parameter STAGES = 3;
parameter DECIMATION = 10;  
parameter IN_WIDTH = 18;

//computed parameters
//ACC_WIDTH = IN_WIDTH + Ceil(STAGES * Log2(DECIMATION))
//OUT_WIDTH = IN_WIDTH + Ceil(Log2(DECIMATION) / 2)
parameter ACC_WIDTH = IN_WIDTH  + 10;
parameter OUT_WIDTH = IN_WIDTH  + 2; // Hermes only uses ADC input width plus 2 here

input clock;
input in_strobe;
output reg out_strobe;
input signed [IN_WIDTH-1:0] in_data;
output signed [OUT_WIDTH-1:0] out_data;

//------------------------------------------------------------------------------
//                               control
//------------------------------------------------------------------------------
reg [15:0] sample_no;
initial sample_no = 15'd0;


always @(posedge clock)
  if (in_strobe)
    begin
    if (sample_no == (DECIMATION-1))
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

    cic_integrator #(ACC_WIDTH) cic_integrator_inst(
      .clock(clock),
      .strobe(in_strobe),
      .in_data(integrator_data[i]),
      .out_data(integrator_data[i+1])
      );


    cic_comb #(ACC_WIDTH) cic_comb_inst(
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
assign out_data = comb_data[STAGES][ACC_WIDTH-1:ACC_WIDTH-OUT_WIDTH] +
  {{(OUT_WIDTH-1){1'b0}}, comb_data[STAGES][ACC_WIDTH-OUT_WIDTH-1]};

//assign out_data = comb_data[STAGES][36:19] + comb_data[STAGES][18];


endmodule

