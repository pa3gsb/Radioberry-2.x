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


//  Metis code copyright 2010, 2011, 2012, 2013 Phil Harman VK6APH, Alex Shovkoplyas, VE3NEA.


module ip_recv(
  //input data stream
  input clock, 
  input rx_enable,
  input [7:0] data,
  input broadcast,
  input [31:0] local_ip,

  //output
  output active,
  output reg is_icmp,
  output reg [31:0] remote_ip,
  output reg [31:0] to_ip
  );

  

  

localparam ST_IDLE = 4'd1, ST_HEADER = 4'd2, ST_PAYLOAD = 4'd4, ST_DONE = 4'd8;
reg[3:0] state;
reg [10:0] header_len, packet_len, byte_no;
reg [31:0] temp_remote_ip;

assign active = rx_enable & (state == ST_PAYLOAD);


always @(posedge clock)
  if (rx_enable)
    case (state)
      ST_IDLE:
        begin
        //save header length
        header_len <= {data[3:0], 2'b0};
        byte_no <= 11'd2;												// need to skip the next byte
        //is protocol = IPv4?
        state <= (data[7:4] == 4'h4)? ST_HEADER : ST_DONE;
        end
    
      ST_HEADER:
        begin        
        case (byte_no)
          //save packet length
			  3: packet_len[10:8] <= data [2:0];	
			  4: packet_len[7:0]  <= data;

			  //determine the protocol
			 10: if (data == 8'd1) is_icmp <= 1'b1;
				  else if (data == 8'h11)   // then will be udp
						is_icmp <= 1'b0;
				  else state <= ST_DONE;	  // neither so exit
            
			    //save sender's ip
			 13: temp_remote_ip[31:24] <= data;
			 14: temp_remote_ip[23:16] <= data;
			 15: temp_remote_ip[15:8]  <= data;
			 16: temp_remote_ip[7:0]   <= data;
				
          //verify broadcast - or save to_ip		 
	  17: begin 
	     remote_ip <= temp_remote_ip;
	     if (broadcast) begin 
		if (data != 8'd255 && data != local_ip[31-:8]) state <= ST_DONE;  
	     end
	     else  to_ip[31-:8] <= data;  // save the ip address this packet is addressed to
	  end
	  
	  18: if (broadcast) begin
	     if (data != 8'd255 && data != local_ip[23-:8]) state <= ST_DONE;
	  end 
	  else  to_ip[23-:8] <= data;
	  
	  19: if (broadcast) begin 
	     if (data != 8'd255 && data != local_ip[15-:8]) state <= ST_DONE;
	  end 
	  else  to_ip[15-:8] <= data;
	  
	  
	  20: 	if (broadcast) begin
	     if(data != 8'd255) state <= ST_DONE; 
	     else if (byte_no == header_len) begin
		byte_no <= 11'd1;
		state <= ST_PAYLOAD;
	     end
	  end				
	  else begin
	     to_ip[7-:8] <= data;
	     if (byte_no == header_len) begin
		byte_no <= 11'd1;
		state <= ST_PAYLOAD;
	     end 
	  end 
          
          default
            if (byte_no == header_len) state <= ST_PAYLOAD;
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
    state <= ST_IDLE;
    
  
  
endmodule
  
