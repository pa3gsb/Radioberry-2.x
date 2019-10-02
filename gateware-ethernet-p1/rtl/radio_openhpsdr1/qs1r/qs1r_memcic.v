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



/*
--------------------------------------------------------------------------------
Memory-based CIC decimator
--------------------------------------------------------------------------------
memory: 2 x M9K blocks
input 24-bit
output 24-bit
internal registers 72-bit
decimation <= 255
stages <= 16
F_in_strobe <= F_clock / (2*stages + 5)
in_strobe length = 1 clock cycle
--------------------------------------------------------------------------------
*/


module qs1r_memcic(
  input clock,
  input in_strobe,
  output reg out_strobe,
  input signed [23:0] in_data,
  output reg signed [23:0] out_data 
  );


parameter STAGES = 11;
parameter DECIMATION = 20; 
parameter ACC_WIDTH = 72; //must be <= 72

localparam MSB = ACC_WIDTH - 1;
localparam LSB = ACC_WIDTH - 24;




//------------------------------------------------------------------------------
//                             control regs
//------------------------------------------------------------------------------
reg [2:0] state;
wire is_comb = state[2];

reg [3:0] rdaddress; 
reg [3:0] wraddress; 
reg wren;
reg [7:0] sample_no;


initial
  begin
  state = 0;
  sample_no = 0;
  rdaddress = 0;
  wraddress = 0;
  wren = 0;
  out_strobe = 0; 
  end






//------------------------------------------------------------------------------
//                             computations
//------------------------------------------------------------------------------
reg signed [ACC_WIDTH-1:0] work_reg;


wire signed [ACC_WIDTH-1:0] new_intg_value = work_reg + ram_output;
wire signed [ACC_WIDTH-1:0] new_comb_value = work_reg - ram_output;


wire signed [ACC_WIDTH-1:0] ram_input = is_comb ? work_reg : new_intg_value;
wire signed [ACC_WIDTH-1:0] ram_output;

wire [71:ACC_WIDTH] zeros = 0;




//------------------------------------------------------------------------------
//                                 loop
//------------------------------------------------------------------------------
always @(posedge clock)
  case (state)

    //integrators
    //-----------

    0:                                
      begin
      out_strobe <= 0;

      if (in_strobe)
        begin
        work_reg <= in_data;
        rdaddress <= 1;
        state <= 1;
        end
      end

    1:                                
      begin
      rdaddress <= 2;
      wraddress <= 0;
      wren <= 1;
      state <= 2;
      end

    2:                                
      begin
      work_reg <= new_intg_value; 

      if (wraddress < (STAGES-1))
        begin
        rdaddress <= rdaddress + 4'd1;
        wraddress <= wraddress + 4'd1;
        end
      else
        begin
        wren <= 0;
        rdaddress <= 0;

        if (sample_no < (DECIMATION-1)) 
            begin 
            sample_no <= sample_no + 8'd1; 
            state <= 0;
            end
          else 
            begin 
            sample_no <= 0;
            state <= 4;
            end
        end
      end 


    //combs
    //-----

    4:
      begin
      rdaddress <= 1;
      state <= 5;
      end

    5:                               
      begin
      rdaddress <= 2;
      wraddress <= 0;
      wren <= 1;
      state <= 6;     
      end

    6:
      begin
      work_reg <= new_comb_value;

      if (wraddress < (STAGES-1))
        begin
        rdaddress <= rdaddress + 4'd1;
        wraddress <= wraddress + 4'd1;
        end
      else
        begin
        wren <= 0;
        state <= 7;
        end
      end

    7:
      begin
      out_data <= work_reg[MSB:LSB] + work_reg[LSB-1];
      out_strobe <= 1;
      rdaddress <= 0;
      state <= 0;
      end

    endcase






//------------------------------------------------------------------------------
//                                    RAM 
//------------------------------------------------------------------------------
//ram output data are available 3 cycles after the address counter change
//if ACC_WIDTH < 72, a few high bits of q() will be left dongling
qs1r_memcic_ram	memcic_ram_inst(
  .clock(clock),
  .rdaddress({is_comb, rdaddress}),
  .wraddress({is_comb, wraddress}),
  .wren(wren),
  .data({zeros, ram_input}),
  .q(ram_output)
  );



endmodule

