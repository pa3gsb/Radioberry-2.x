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

//------------------------------------------------------------------------------
//           Copyright (c) 2013,2015 Phil Harman, VK6(A)PH 
//------------------------------------------------------------------------------

// 2013 Jan 26 - varcic now accepts 2...40 as decimation and CFIR
//               replaced with Polyphase FIR - VK6APH
// 2015 Apr 20 - cic now by Jeremy McDermond, NH6Z
//                  - single polyphase FIR Filter
//      Jul 25 - add reset to CORDIC, CIC and FIR for Sync operation



module receiver2(
  input reset,
  input clock,                  //122.88 MHz
  input [31:0] frequency,
  input [5:0] sample_rate,
  output out_strobe,
  input signed [15:0] in_data,
  output signed [23:0] out_data_I,
  output signed [23:0] out_data_Q
  );

wire signed [21:0] cordic_outdata_I;
wire signed [21:0] cordic_outdata_Q;
reg [5:0] rate0, rate1;

//------------------------------------------------------------------------------
//                               cordic
//------------------------------------------------------------------------------

recv2_cordic recv2_cordic_inst(
  .reset(reset),
  .clock(clock),
  .in_data(in_data),             //16 bit 
  .frequency(frequency),         //32 bit
  .out_data_I(cordic_outdata_I), //22 bit
  .out_data_Q(cordic_outdata_Q)
  );

  
 
// Select CIC decimation rates based on sample_rate
// Sample rate encoded as per receiver1

    always @ (sample_rate)              
    begin 
        case (sample_rate)  
          6'd40: begin rate0 <= 6'd32;  rate1 <= 6'd25; end  //48K
          6'd20: begin rate0 <= 6'd16;  rate1 <= 6'd25; end  //96K       
          6'd10: begin rate0 <= 6'd8;   rate1 <= 6'd25; end  //192K  
          6'd5: begin rate0 <=  6'd4;   rate1 <= 6'd25; end //384K      
          default: begin rate0 <= 6'd32; rate1 <= 6'd25; end
        endcase
    end 

  
// Receive CIC filters followed by FIR filter
wire decimA_avail, decimB_avail;
wire signed [17:0] decimA_real;
wire signed [17:0] decimA_imag;
wire signed [23:0] decimB_real, decimB_imag;

wire cic_outstrobe_2;
wire signed [23:0] cic_outdata_I2;
wire signed [23:0] cic_outdata_Q2;

//I channel
recv2_cic #(.STAGES(3), .MIN_DECIMATION(4), .MAX_DECIMATION(32), .IN_WIDTH(22), .OUT_WIDTH(18))
 cic_inst_I2(.reset(1'b0),
                 .decimation(rate0),
                 .clock(clock), 
                 .in_strobe(1'b1),
                 .out_strobe(decimA_avail),
                 .in_data(cordic_outdata_I),
                 .out_data(decimA_real)
                 );
                 
//Q channel
recv2_cic #(.STAGES(3), .MIN_DECIMATION(4), .MAX_DECIMATION(32), .IN_WIDTH(22), .OUT_WIDTH(18)) 
 cic_inst_Q2(.reset(1'b0),
                 .decimation(rate0),
                 .clock(clock), 
                 .in_strobe(1'b1),
                 .out_strobe(),
                 .in_data(cordic_outdata_Q),
                 .out_data(decimA_imag)
                 );         
            

wire cic_outstrobe_1;
wire signed [22:0] cic_outdata_I1;
wire signed [22:0] cic_outdata_Q1;


recv2_cic #(.STAGES(11), .MIN_DECIMATION(8), .MAX_DECIMATION(32), .IN_WIDTH(18), .OUT_WIDTH(24)) 
 varcic_inst_I1(.reset(1'b0),
                 .decimation(rate1),
                 .clock(clock), 
                 .in_strobe(decimA_avail),
                 .out_strobe(decimB_avail),
                 .in_data(decimA_real),
                 .out_data(decimB_real)
                 );
                 

//Q channel
recv2_cic #(.STAGES(11), .MIN_DECIMATION(8), .MAX_DECIMATION(32), .IN_WIDTH(18), .OUT_WIDTH(24)) 
 varcic_inst_Q1(.reset(1'b0),
                 .decimation(rate1),
                 .clock(clock), 
                 .in_strobe(decimA_avail),
                 .out_strobe(),
                 .in_data(decimA_imag),
                 .out_data(decimB_imag)
                 );
                 
        
// Polyphase decimate by 2 FIR Filter
recv2_firX2R2 fir3 (1'b0, clock, decimB_avail, decimB_real, decimB_imag, out_strobe, out_data_I, out_data_Q);


endmodule
