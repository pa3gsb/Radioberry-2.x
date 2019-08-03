//
//  HPSDR - High Performance Software Defined Radio
//
//  Metis code. 
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


//  Metis code copyright 2010, 2011, 2012, 2013 Alex Shovkoplyas, VE3NEA.

//  25 Sept 2014 - Modified initial register values to correct for 0.12nS rather than 0.2nS steps.
//                 Also added write to register 106h to turn off Tx Data Pad Skews.  Both these
//						 changes are due to errors in the original data sheet which was corrected Feb 2014.


module phy_cfg(
  //input
  input clock,        //2.5 MHZ
  input init_request,
  
  //output
  output reg [1:0] speed,
  output reg duplex,
  
  //hardware pins
  inout mdio_pin,
  output mdc_pin  
);


//-----------------------------------------------------------------------------
//                           initialization data
//-----------------------------------------------------------------------------

//mdio register values
wire [15:0] values [2:0];

assign values[2] = 16'b0010000100000000;
assign values[1] = 16'b1000000000000000;
assign values[0] = 16'hxxxx;


//mdio register addresses 
wire [4:0] addresses [2:0];

assign addresses[2] = 0;
assign addresses[1] = 0;
assign addresses[0] = 31; 

reg [1:0] word_no; 






//-----------------------------------------------------------------------------
//                            state machine
//-----------------------------------------------------------------------------


reg init_required;

wire ready;
wire [15:0] rd_data;
reg rd_request, wr_request;


//state machine  
localparam READING = 1'b0, WRITING = 1'b1;  
reg state = READING;  


always @(posedge clock)  
  begin
  if (init_request) 
    init_required <= 1;
  
  if (ready)
    case (state)
      READING:
        begin
        speed <= rd_data[3:2];
        duplex <= rd_data[4];
        
        if (init_required)
          begin
          wr_request <= 1;
          word_no <= 2;
          state  <= WRITING;
          init_required <= 0;
          end
        else
          rd_request <= 1'b1;
        end

      WRITING:
        begin
        if (word_no == 2'b1) state <= READING;   // *** should this be == 0?
        else wr_request <= 1;
        word_no <= word_no - 2'b1;		  
        end
      endcase
		
  else //!ready
    begin
    rd_request <= 0;
    wr_request <= 0;
    end
  end

        
        
        
        
//-----------------------------------------------------------------------------
//                        MDIO interface to PHY
//-----------------------------------------------------------------------------


mdio mdio_inst (
  .clock(clock), 
  .addr(addresses[word_no]), 
  .rd_request(rd_request),
  .wr_request(wr_request),
  .ready(ready),
  .rd_data(rd_data),
  .wr_data(values[word_no]),
  .mdio_pin(mdio_pin),
  .mdc_pin(mdc_pin)
  );  
  



  
  
endmodule
