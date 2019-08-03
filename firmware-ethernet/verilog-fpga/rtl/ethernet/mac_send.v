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




module mac_send (
  input clock, 
  input reset,
  input tx_enable,
  output active,

  input [7:0] data_in,
  output [7:0] data_out,
  input [47:0] local_mac,
  input [47:0] destination_mac
  );
  
//tx bits and shift register
localparam MIN_PACKET_LEN = 6'd46;
localparam HEADER_LEN = 6'd14;
localparam SHR_LEN = 6'd12;
localparam HI_SHR_BIT = 12*8 - 1;
wire [HI_SHR_BIT:0] tx_bits = {destination_mac, local_mac};
reg [HI_SHR_BIT:0] shift_reg;
assign data_out = shift_reg[HI_SHR_BIT -: 8];
reg [5:0] byte_no;  


//crc32 calculation
wire [31:0] crc;  

crc32 crc32_inst (
  .clock(clock),
  .clear(!active),
  .enable(1'b1),
  .data(data_out),
  .result(crc)
  );  
  
  
//state
localparam ST_IDLE = 1, ST_HEADER = 2, ST_PAYLOAD = 4, ST_TAIL = 8, ST_CRC = 16;
reg [4:0] state = ST_IDLE;  


assign active = !reset && (tx_enable || (state != ST_IDLE));


always @(posedge clock)  
  if (reset) state <= ST_IDLE;
    
  else    
    begin
    //shift register
    if ((state == ST_IDLE) && !tx_enable) shift_reg <= tx_bits;
    else if ((state == ST_TAIL) && (byte_no == 0)) shift_reg[HI_SHR_BIT -: 32] <= {crc[7-:8],crc[15-:8],crc[23-:8],crc[31-:8]};
    else if (tx_enable) shift_reg <= {shift_reg[HI_SHR_BIT-8:0], data_in};
    else shift_reg <= {shift_reg[HI_SHR_BIT-8:0], 8'b0};


    case (state)
      //waitfor tx_enable to go high
      ST_IDLE:
        if (tx_enable) 
          begin
          byte_no <= HEADER_LEN-6'd2;
          state <= ST_HEADER;
          end
          
      ST_HEADER:
        //push header out of shr
        if (byte_no != 0) 
          byte_no <= (byte_no - 6'd1);
        //header sent, keep pushing payload into shr
        else
          begin
          byte_no <= (MIN_PACKET_LEN - SHR_LEN);// - 6'd1);
          state <= ST_PAYLOAD;
          end
        
      ST_PAYLOAD:
        //read into shr at least MIN_PACKET_LEN bytes
        if (byte_no != 0) byte_no <= byte_no - 6'd1; 
        //no more payload bytes
        else if (!tx_enable) 
          begin 
          byte_no <= SHR_LEN-6'd2; 
          state <= ST_TAIL;
          end
          
      ST_TAIL:
        //purge shr
        if (byte_no != 0) byte_no <= byte_no - 6'd1;
        //all bytes sent, append crc
        else
          begin 
          byte_no <= 4'd3; 
          state <= ST_CRC; 
          end
      
      ST_CRC:
        //push out 4 bytes of crc         
        if (byte_no != 0) byte_no <= byte_no - 6'd1;
        else state <= ST_IDLE;        
    endcase
  end
  
  
endmodule
