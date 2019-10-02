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
// receive ethernet header; check destination mac address and discard packet
// if it is not for us; extract source mac address and packet type
//-----------------------------------------------------------------------------

module mac_recv(
  //input data stream
  input clock, 
  input rx_enable,
  input [7:0] data,

  //constant input parameter
  input [47:0] local_mac,

  //output
  output active,
  output reg broadcast,
  output reg is_arp,
  output reg [47:0] remote_mac
  );

  

  


//-----------------------------------------------------------------------------
//                            state machine
//-----------------------------------------------------------------------------
localparam ST_DST_ADDR = 5'd1, ST_SRC_ADDR = 5'd2, ST_PROTO = 5'd4, ST_PAYLOAD = 5'd8, ST_ERROR = 5'd16;
reg[4:0] state;

reg [2:0] byte_no;
reg unicast;
reg [47:0] temp_remote_mac;

localparam HI_MAC_BYTE = 3'd5, HI_PROTO_BYTE = 3'd1;
localparam false = 1'b0, true = 1'b1;

assign active = rx_enable & (state == ST_PAYLOAD);


always @(posedge clock)
  if (rx_enable)
    case (state)
      ST_DST_ADDR:
        begin
        //is broadcast addr
        if (data != 8'hFF) broadcast <= false;
        //is local mac addr
        if (data != local_mac[byte_no*8+7 -: 8]) unicast <= false; 
        //continue
        if (byte_no != 0) byte_no <= byte_no - 3'd1; 
        //dst addr done
        else begin byte_no <= HI_MAC_BYTE; state <= ST_SRC_ADDR; end
        end
        
      ST_SRC_ADDR:
        begin        
			  //save remote mac
			  temp_remote_mac <= {temp_remote_mac[39:0], data};
			  if (byte_no != 0) byte_no <= byte_no - 3'd1;
			  //good destination mac, protocol id follows
			  else if (broadcast | unicast) begin byte_no <= HI_PROTO_BYTE; state <= ST_PROTO; end
			  //bad destination mac, discard message
			  else state <= ST_ERROR;        
        end
        
      ST_PROTO:
			begin 
			  //protocol 0806 = arp, 0800 = ip
			  if (byte_no != 0) 
				 if (data != 8'h08) state <= ST_ERROR; 
				 else byte_no <= byte_no - 3'd1;
			  else if (data == 8'h06) begin
			  	 is_arp <= true; 
				 remote_mac <= temp_remote_mac;  // only update mac if protocol valid
				 state <= ST_PAYLOAD;
			  end
			  else if (data == 8'h00) begin
			  	 is_arp <= false;
				 remote_mac <= temp_remote_mac;  // only update mac if protocol vaild
				 state <= ST_PAYLOAD;
			  end
			  else state <= ST_ERROR;
		  end
      endcase
      
  else //!rx_enable 
    begin
    broadcast <= true;
    unicast <= true;
    byte_no <= HI_MAC_BYTE;
    state <= ST_DST_ADDR;
    end
    
  
  
endmodule
  
