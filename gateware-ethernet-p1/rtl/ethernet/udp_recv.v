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


//  Metis code copyright 2010, 2011, 2012, 2013, 2014 Phil Harman VK6(A)PH, Alex Shovkoplyas, VE3NEA.

module udp_recv(
  //input data stream
  input clock, 
  input rx_enable,
  input [7:0] data,
  input [31:0] to_ip,
  input broadcast,
  input [47:0] remote_mac,
  input [31:0] remote_ip,

  //constant input parameter
  input [31:0] local_ip,

  //output
  output active,
  output dhcp_active,				 
  output reg [15:0] to_port,					// port that data is being sent to.
  output reg [31:0] udp_destination_ip,   
  output reg [47:0] udp_destination_mac,
  output reg [15:0] udp_destination_port
  );

  

  

localparam IDLE = 4'd1, PORT = 4'd2, VERIFY = 4'd3, ST_PAYLOAD = 4'd4, ST_DONE = 4'd5;
reg[3:0] state;
reg [10:0] header_len, packet_len, byte_no;
reg dhcp_data;
reg [15:0] remote_port;

assign active      = rx_enable & (state == ST_PAYLOAD) & !dhcp_data;
assign dhcp_active = rx_enable & (state == ST_PAYLOAD) & dhcp_data;

always @(posedge clock)
  if (rx_enable)
    case (state)
	  IDLE:
        begin
        //save remote port address
        remote_port[15:8] <= data; 
        state <= PORT;
		  dhcp_data <= 1'b0;   
        end
		  
		PORT:
			begin
			remote_port <= {remote_port[15:8], data};
			byte_no <= 11'd3;
			state <= VERIFY;
			end 
	 
      VERIFY:
        begin        
			  case (byte_no)
				 // get the port the packet is addressed to
			    3:  to_port[15:8] <= data;
				 4:  to_port[7:0]  <= data;
			 
	           // verify DHCP, broadcast to port 1024  or the ip address its being sent to then save packet length				 
				 5: begin 
						if (to_port == 16'd68) dhcp_data <= 1'b1;				// check for DHCP data
						else if (broadcast) begin
								if (to_port != 16'd1024) state <= ST_DONE;
						end 
						else if (local_ip != to_ip ) state <= ST_DONE; 			// if not for this  ip then exit
						packet_len[10:8] <= data[2:0];
					 end
					 
             6: packet_len[7:0] <= data;  
				 
				 // skip the checksum then signal we have a udp packet available,save destination IP, MAC and port
				 8: begin
						udp_destination_ip   <= remote_ip;  
						udp_destination_mac  <= remote_mac;
						udp_destination_port <= remote_port;
						state <= ST_PAYLOAD;
					 end 
				  
				// default:  *** need to get this from ip_recv
				//	if (byte_no == header_len) state <= ST_PAYLOAD;
			  endcase  
			  
        byte_no <= byte_no + 11'd1;
        end
        
      ST_PAYLOAD:
        begin  
        //end of payload, ignore the ethernet crc that follows
        if (byte_no == packet_len) state <= ST_DONE;
        byte_no <= byte_no + 11'd1;
        end        
      endcase
      
  else //!rx_enable 
    state <= IDLE;
    
  
  
endmodule	