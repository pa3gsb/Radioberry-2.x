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


//-----------------------------------------------------------------------------
// read/write the registers of a PHY device using its MDIO interface
//
// set addr, wr_data and start_xx on posedge of clock
// keep addr and wr_data valid until ready changes to high
//-----------------------------------------------------------------------------

module mdio(
  //control
  input clock,
  input [4:0]addr,
  input rd_request,
  input wr_request,
  output ready,
  
  //data
  input [15:0] wr_data,
  output reg [15:0] rd_data,  
  
  //hardware pins
  inout mdio_pin,
  output mdc_pin
  );


//bits to send

wire [63:0] wr_bits = {32'hFFFFFFFF, 9'b010100001, addr, 2'b10, wr_data};  // PHYAD[4:0]=5'h7
wire [63:0] rd_bits = {32'hFFFFFFFF, 9'b011000001, addr, 2'bxx, 16'hFFFF}; // PHYAD[4:0]=5'h7


reg[ 5:0] bit_no;
  

//state  
localparam ST_IDLE = 3'd1, ST_READING = 3'd2, ST_WRITING = 3'd4;  
reg[2:0] state = ST_IDLE; 


//3-state mdio pin
reg mdio_high_z;
assign mdio_pin = mdio_high_z ? 1'bz : (state == ST_READING ? rd_bits[bit_no] : wr_bits[bit_no]);


//state machine  
always @(negedge clock)  
  case (state)
    ST_IDLE:
      begin
      mdio_high_z <= 0;
      bit_no <= 63; 
      if (rd_request) state <= ST_READING;
      else if (wr_request) state <= ST_WRITING;
      end

    ST_READING:
      begin
      if (bit_no == 18) mdio_high_z <= 1;
      rd_data <= {rd_data[14:0], mdio_pin};
      if (bit_no == 1) state <= ST_IDLE;
      bit_no <= bit_no - 6'b1;
      end

    ST_WRITING:
      begin
      if (bit_no == 0) state <= ST_IDLE; 
      bit_no <= bit_no - 6'b1;
      end

    default
      state <= ST_IDLE;
  endcase  

  


//control 
assign mdc_pin = clock;
assign ready = (state == ST_IDLE);






endmodule
