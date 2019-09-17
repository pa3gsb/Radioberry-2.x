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


//-----------------------------------------------------------------------------
// decode arp packet, generate reply
//-----------------------------------------------------------------------------

module arp (
  input reset, //when PHY not connected
  input rx_clock,  
  input rx_enable,  
  input [7:0] rx_data,
  input tx_clock, 
  input [47:0] local_mac, 
  input [31:0] local_ip,
  input [47:0] remote_mac,
  input tx_enable,  
 
  output [7:0] tx_data,
  output reg [47:0] destination_mac,  
  output tx_request,   
  output tx_active
);






//-----------------------------------------------------------------------------
//                                receive
//-----------------------------------------------------------------------------
//rx_bits = {80'h_0806_0001_0800_0604_0001, destination_mac, 32'bx, 48'bx, local_ip};


localparam ST_IDLE = 5'd1, ST_RX = 5'd2, ST_TXREQ = 5'd4, ST_TX = 5'd8, ST_ERR = 5'd16; 
reg [4:0] state = ST_IDLE;


localparam RX_LEN = 5'd28;
reg [4:0] byte_no;
reg [31:0] remote_ip;

always @(posedge rx_clock)
  case (state)
    ST_IDLE:
      //rx_enable is high, start reception
      if (rx_enable) 
        begin 
        destination_mac <= remote_mac; 
        byte_no <= RX_LEN - 5'd2;
        state <= ST_RX; 
        end
         
    ST_RX:
      //rx_enable low, abort reception
      if (!rx_enable) state <= ST_IDLE;      
      else
        begin
        case (byte_no)
          //arp operation, h0001 = request
          21: if (rx_data != 8'h00) state <= ST_ERR;
          20: if (rx_data != 8'h01) state <= ST_ERR;
			 
//			 13: remote_ip[31:24]  <= rx_data;
//			 12: remote_ip[23:16]  <= rx_data;
//			 11: remote_ip[15:8]   <= rx_data;
//			 10: remote_ip[7:0]    <= rx_data;
            
//          //save sender's ip 
          10,11,12,13: remote_ip <= {remote_ip[23:0], rx_data};  

//			  3: if (rx_data != local_ip[31-:8]) state <= ST_ERR;
//			  2: if (rx_data != local_ip[31-:8]) state <= ST_ERR;
//			  1: if (rx_data != local_ip[31-:8]) state <= ST_ERR;
//			  0: begin 
//					if (rx_data != local_ip[31-:8]) state <= ST_ERR;
//               else state <= ST_TXREQ; 
//			     end 	
          0,1,2,3: 
            //compare target ip to our local_ip     
            if (rx_data != local_ip[byte_no*8+7 -:8]) state <= ST_ERR;
            //packet received, request permission to send reply
            else if (byte_no == 0) state <= ST_TXREQ;      
          endcase    
          
         byte_no <= byte_no - 5'd1;
         end
      
    //wait for permission to send
    ST_TXREQ: if (sending) state <= ST_TX;    

    //wait for the end of sending
    ST_TX: if (!sending) state <= ST_IDLE;
    
    //end of discarded packet
    ST_ERR: if (!rx_enable) state <= ST_IDLE;
  endcase
        
    


  
  
//-----------------------------------------------------------------------------
//                                  send
//-----------------------------------------------------------------------------
//message to send
localparam TX_LEN = 5'd30;
wire [TX_LEN*8-1:0] tx_bits = {80'h_0806_0001_0800_0604_0002, local_mac, local_ip, destination_mac, remote_ip};
reg [4:0] tx_byte_no;
assign tx_data = tx_bits[tx_byte_no*8+7 -:8];
  

assign tx_request = (state == ST_TXREQ);


//transfer {sending} to rx clock domain
localparam false = 1'b0, true = 1'b1;
reg sending = false;



assign tx_active = tx_enable | sending;


always @(posedge tx_clock)
  begin
  //reset is high, stop sending
  if (reset) sending <= false;
  //permission granted, start sending
  else if (tx_enable) sending <= true;
  
  if (tx_active)   
    //last byte sent
    if (tx_byte_no == 5'd0) sending <= false;
    //send next byte
    else tx_byte_no <= tx_byte_no - 5'd1;
  else
    //load data to send
    tx_byte_no <= TX_LEN - 5'd1;
  end
    

  
endmodule
