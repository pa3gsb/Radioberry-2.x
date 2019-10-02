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



module qs1r_fir( clock, start, coeff, in_data, out_data, out_strobe );

parameter OUT_WIDTH = 32;
localparam MSB = 46;
localparam LSB = MSB - OUT_WIDTH + 1;

input clock;
input start;
input signed [23:0] coeff;
input signed [23:0] in_data;
output reg signed [OUT_WIDTH-1:0] out_data;
output reg out_strobe;


reg [2:0] state;
reg shift;
reg clear_mac;
reg even_sample;
wire last_sample;
wire [23:0] shr_out;
wire signed [55:0] mac_out;

initial
  begin
  shift = 0;
  clear_mac = 1; 
  state = 0;
  even_sample = 1;
  end


always @(posedge clock)
    case (state)
      0: //if start=1: write new sample to shiftreg, dump the oldest sample
        if (start) state <= state + 1'b1;

      1: //clear mac
        begin         
        clear_mac <= 1;
        state <= state + 1'b1;
        end
        
      2: //switch shiftreg to shift mode; enable mac
        begin         
        shift <= 1;
        clear_mac <= 0;
        state <= state + 1'b1;
        end
      

      3: //strobe 256 sample/coeff pairs into mac, then stop shifting
        if (last_sample) 
          begin
          shift <= 0;
          state <= state + 1'b1;
          end
          
      4, 5: //wait for the mac pipeline to finish
        state <= state + 1'b1;
        
      6: //round and register mac output, strobe every even out_data
        begin       
        out_data <= mac_out[MSB:LSB] + mac_out[LSB-1];
        if (even_sample) out_strobe <= 1;
        even_sample <= ~even_sample;
        state <= state + 1'b1;
        end

      7: //done, clear strobe
        begin
        out_strobe <= 0;     
        state <= 0;
        end

    endcase


//------------------------------------------------------------------------------
//                    circular shift register 256 x 24 bit
//------------------------------------------------------------------------------
wire [23:0] shr_in = start ? in_data : shr_out;


qs1r_fir_shiftreg fir_shiftreg_inst(
  .clock(clock),
  .clken(start | shift),
  .shiftin({start, shr_in}), //MSB bit flags the new sample
  .shiftout({last_sample, shr_out})
  );

//------------------------------------------------------------------------------
//                        multiplier / accumulator
//------------------------------------------------------------------------------
qs1r_fir_mac fir_mac_inst(
  .clock(clock),
  .clear(clear_mac),
  .in_data_1(shr_out),
  .in_data_2(coeff),
  .out_data(mac_out)
  );

endmodule
