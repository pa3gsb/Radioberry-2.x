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

// 2013 Jan 26	- Modified to accept decimation values from 1-40. VK6APH 

module varcic(decimation, clock, in_strobe,  out_strobe, in_data, out_data );

  //design parameters
  parameter STAGES = 5;
  parameter IN_WIDTH = 18;
  parameter ACC_WIDTH = 45;
  parameter OUT_WIDTH = 18;
  parameter CICRATE = 12;
  
  input [5:0] decimation; 
  
  input clock;
  input in_strobe;
  output reg out_strobe;

  input signed [IN_WIDTH-1:0] in_data;
  output reg signed [OUT_WIDTH-1:0] out_data;


//------------------------------------------------------------------------------
//                               control
//------------------------------------------------------------------------------
reg [15:0] sample_no = 0;

always @(posedge clock)
  if (in_strobe)
    begin
    if (sample_no == (decimation - 1))
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

/*
-----------------------------------------------------
 Output rounding calculations for 5 stages 

 sample rate (ksps)  decimation 	 bit growth
  48                  16            34-14 = 20
  96                   8            29-14 = 15
 192                   4            24-14 = 10
 384                   2            19-14 = 5
-------------------------------------------------------		  
*/		
// also math.log(growthN,2) * 5

// FIXME: Should be cleaner way to do all this...
localparam GROWTH2  =  5;
localparam GROWTH4  = 10;
localparam GROWTH8  = 15;
localparam GROWTH16 = 20;

localparam MSB2  =  (IN_WIDTH + GROWTH2)  - 1;           
localparam LSB2  =  (IN_WIDTH + GROWTH2)  - OUT_WIDTH;   

localparam MSB4  =  (IN_WIDTH + GROWTH4)  - 1;            
localparam LSB4  =  (IN_WIDTH + GROWTH4)  - OUT_WIDTH;  

localparam MSB8  =  (IN_WIDTH + GROWTH8)  - 1;           
localparam LSB8  =  (IN_WIDTH + GROWTH8)  - OUT_WIDTH;  

localparam MSB16 =  (IN_WIDTH + GROWTH16) - 1;       
localparam LSB16 =  (IN_WIDTH + GROWTH16) - OUT_WIDTH;


localparam GROWTH3  =  8;
localparam GROWTH6  = 13;
localparam GROWTH12  = 18;
localparam GROWTH24 = 23;

localparam MSB3  =  (IN_WIDTH + GROWTH3)  - 1;           
localparam LSB3  =  (IN_WIDTH + GROWTH3)  - OUT_WIDTH;   

localparam MSB6  =  (IN_WIDTH + GROWTH6)  - 1;            
localparam LSB6  =  (IN_WIDTH + GROWTH6)  - OUT_WIDTH;  

localparam MSB12  =  (IN_WIDTH + GROWTH12)  - 1;           
localparam LSB12  =  (IN_WIDTH + GROWTH12)  - OUT_WIDTH;  

localparam MSB24 =  (IN_WIDTH + GROWTH24) - 1;       
localparam LSB24 =  (IN_WIDTH + GROWTH24) - OUT_WIDTH;


localparam GROWTH5  =  12;
localparam GROWTH10  = 17;
localparam GROWTH20  = 22;
localparam GROWTH40 = 27;

localparam MSB5  =  (IN_WIDTH + GROWTH5)  - 1;           
localparam LSB5  =  (IN_WIDTH + GROWTH5)  - OUT_WIDTH;   

localparam MSB10  =  (IN_WIDTH + GROWTH10)  - 1;            
localparam LSB10  =  (IN_WIDTH + GROWTH10)  - OUT_WIDTH;  

localparam MSB20  =  (IN_WIDTH + GROWTH20)  - 1;           
localparam LSB20  =  (IN_WIDTH + GROWTH20)  - OUT_WIDTH;  

localparam MSB40 =  (IN_WIDTH + GROWTH40) - 1;       
localparam LSB40 =  (IN_WIDTH + GROWTH40) - OUT_WIDTH;

generate
  if (CICRATE == 10)
    always @(posedge clock)
      case (decimation)
         2: out_data <= comb_data[STAGES][MSB2:LSB2]   + comb_data[STAGES][LSB2-1];
         4: out_data <= comb_data[STAGES][MSB4:LSB4]   + comb_data[STAGES][LSB4-1];
         8: out_data <= comb_data[STAGES][MSB8:LSB8]   + comb_data[STAGES][LSB8-1];
        default: out_data <= comb_data[STAGES][MSB16:LSB16] + comb_data[STAGES][LSB16-1];        
      endcase
  else if (CICRATE == 13)
    always @(posedge clock)
      case (decimation)
         2: out_data <= comb_data[STAGES][MSB2:LSB2]   + comb_data[STAGES][LSB2-1];
         4: out_data <= comb_data[STAGES][MSB4:LSB4]   + comb_data[STAGES][LSB4-1];
         8: out_data <= comb_data[STAGES][MSB8:LSB8]   + comb_data[STAGES][LSB8-1];
        default: out_data <= comb_data[STAGES][MSB16:LSB16] + comb_data[STAGES][LSB16-1];        
      endcase
  else if (CICRATE == 5)
    always @(posedge clock)
      case (decimation)
         5: out_data <= comb_data[STAGES][MSB5:LSB5]   + comb_data[STAGES][LSB5-1];
         10: out_data <= comb_data[STAGES][MSB10:LSB10]   + comb_data[STAGES][LSB10-1];
         20: out_data <= comb_data[STAGES][MSB20:LSB20]   + comb_data[STAGES][LSB20-1];
        default: out_data <= comb_data[STAGES][MSB40:LSB40] + comb_data[STAGES][LSB40-1];    
      endcase 
  else
    always @(posedge clock)
      case (decimation)
         3: out_data <= comb_data[STAGES][MSB3:LSB3]   + comb_data[STAGES][LSB3-1];
         6: out_data <= comb_data[STAGES][MSB6:LSB6]   + comb_data[STAGES][LSB6-1];
         12: out_data <= comb_data[STAGES][MSB12:LSB12]   + comb_data[STAGES][LSB12-1];
        default: out_data <= comb_data[STAGES][MSB24:LSB24] + comb_data[STAGES][LSB24-1];        
      endcase
endgenerate    


endmodule

  
