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


module icmp (
  input reset, 
  input rx_clock,  
  input rx_enable,  
  input [7:0] rx_data,
  input tx_clock, 
  input tx_enable,
  input [47:0] remote_mac,
  input [31:0] remote_ip, 
 
  output dst_unreachable,
  output tx_request, 
  output tx_active,
  output reg [7:0] tx_data,
  output reg [15:0] length, 
  output reg [47:0] destination_mac,  
  output reg [31:0] destination_ip
);


   

//-----------------------------------------------------------------------------
//                               receive
//-----------------------------------------------------------------------------
localparam HEADER_LEN = 16'd4;
localparam 
	ST_IDLE 		= 6'd1, 
	ST_HEADER 		= 6'd2, 
	ST_PAYLOAD 		= 6'd3, 
	ST_TXREQ 		= 6'd4, 
	ST_TX 			= 6'd5, 
	ST_DONE 		= 6'd6,
	ST_UNREACHABLE 	= 6'd7;
	
reg[5:0] state = ST_IDLE;
reg [2:0] byte_no;
reg [31:0] sum;				

wire fifo_full;

  
always @(posedge rx_clock)
    case (state)
      ST_IDLE:
      	begin
      	// clear ICMP dest unreachable flag
      	dst_unreachable <= 1'b0;
        //packet start
        if (rx_enable)
          begin 
          byte_no <= 2;
          sum <= 32'h0;
          destination_mac <= remote_mac;
          destination_ip <= remote_ip;
          // detection of ICMP dest/port unreachable packet
          if (rx_data == 8'h03) state <= ST_UNREACHABLE;
          else
            state <= (rx_data == 8'h08) ? ST_HEADER : ST_DONE; 
          end // if (rx_enable)
        end  

      ST_HEADER:    
        //premature end of packet
        if (!rx_enable) state <= ST_IDLE;
        //end of header
        else if (byte_no == 0) begin length <= HEADER_LEN; state <= ST_PAYLOAD; end
        //invalid header
        else if ((byte_no == 2) && (rx_data != 8'h00)) state <= ST_DONE;
        //next header byte
        else byte_no <= byte_no - 3'd1;

      ST_PAYLOAD: begin
        //end of payload data, reply
        if (!rx_enable) state <= ST_TXREQ;
        //not enough space in fifo
        else if (fifo_full) state <= ST_DONE;
        
        //Allow during fifo_full for timing as data is invalid
        if (rx_enable) begin
          //update checksum
          if (length[0]) sum <= sum + {24'b0, rx_data};  // sum is 32 bits so need to add 24 
          else sum <= sum + {16'd0, rx_data, 8'b0};      // need to form 16 bits from two 8 bit values hence toggle using length[0]  
          //count payload bytes
          length <= length + 16'd1;
        end
      end
      //wait for permission to send
      ST_TXREQ: if (sending) state <= ST_TX;    

      //wait for the end of sending
      ST_TX: if (!sending) state <= ST_IDLE;
      
      //end of discarded packet
      ST_DONE: if (!rx_enable) state <= ST_IDLE;
      
      // Detection of ICMP dest/port unreachable packet
      ST_UNREACHABLE:
        begin
          dst_unreachable <= 1'b1;
          state <= ST_DONE;
        end
    endcase
      
        

        
        
        
//-----------------------------------------------------------------------------
//                       storage for payload data
//-----------------------------------------------------------------------------
localparam false = 1'b0, true = 1'b1;
reg sending = false;


wire fifo_clear = state == ST_IDLE;
wire fifo_write = rx_enable && (state == ST_PAYLOAD);
wire fifo_read;
wire fifo_empty;
wire [7:0] fifo_data;

//fifo read and write clocks
//reg rx_clk_en = 1'b0;
//always @(negedge rx_clock)
//  rx_clk_en <= (state == ST_IDLE) || (state == ST_HEADER) || (state == ST_PAYLOAD);
//reg tx_clk_en = 1'b0;
//always @(negedge tx_clock) 
//  tx_clk_en <= sending;  
//wire fifo_clock = rx_clk_en? rx_clock : tx_clk_en? tx_clock : 1'b0; 


        
//icmp_fifo icmp_fifo_inst (
//  .clock(fifo_clock),
//  .data(rx_data),
//  .rdreq(fifo_read),
//  .sclr(fifo_clear),
//  .wrreq(fifo_write),
//  .empty(fifo_empty),
//  .full(fifo_full),
//  .q(fifo_data)
//  );

icmp_fifo icmp_fifo_inst (
  .aclr(fifo_clear),
  .data(rx_data),
  .rdclk(tx_clock),
  .rdreq(fifo_read),
  .wrclk(rx_clock),
  .wrreq(fifo_write),
  .q(fifo_data),
  .rdempty(fifo_empty),
  .wrfull(fifo_full)
);

  
//-----------------------------------------------------------------------------
//                            ip checksum
//-----------------------------------------------------------------------------

// To calculate:  sum all the data and keep the bottom 16 bits, the top bits being the carry bits.
// Add the carry bits to the bottom 16 bits then invert, the result is the check sum.
// No need to add with carry since sum can never be large enough to cause a carry.

wire [15:0] checksum = ~(sum[15:0] + sum[31:16]);



//-----------------------------------------------------------------------------
//                               send
//-----------------------------------------------------------------------------

assign tx_request = (state == ST_TXREQ);

localparam HDR_LEN = 3'd4;
reg [2:0] tx_byte_no;
reg [15:0] delay;


always @(posedge tx_clock)
  begin
  //(sending) flag
  if (reset | fifo_empty ) sending <= false;
  else if (tx_enable) sending <= true;
  //(tx_byte_no) update
  if (!sending) tx_byte_no <= 3'b100;
  else tx_byte_no <= {1'b0,tx_byte_no[2:1]};
  end
      
  
assign tx_active = (tx_enable | sending); // & ~fifo_empty;

assign fifo_read = (tx_byte_no == 3'h0) && sending && tx_active;

assign tx_data = (tx_byte_no[2])? 8'b0 :
                 (tx_byte_no[1])? checksum[15:8] :
                 (tx_byte_no[0])? checksum[7:0] :
                  fifo_data; 

  
endmodule
