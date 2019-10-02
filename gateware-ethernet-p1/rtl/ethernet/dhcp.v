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


//  Metis code copyright 2010, 2011, 2012, 2013 Phil Harman VK6APH
//  April 2016, N2ADR: Added xid, dhcp_seconds_timer


module dhcp (
  //rx in
  input rx_clock,
  input [7:0] rx_data,
  input rx_enable,
  input dhcp_rx_active,
  //rx out
  output reg [31:0] lease,      // DHCP supplied lease time in seconds
  output reg [31:0] server_ip,   // IP address of DHCP server

  //tx in
  input reset,
  input tx_clock,
  input udp_tx_enable,
  input tx_enable,
  input udp_tx_active,
  input [47:0] remote_mac,
  input [31:0] remote_ip,
  input [3:0] dhcp_seconds_timer,
  //tx out
  output reg dhcp_tx_request,
  output reg[7:0] tx_data,
  output reg [15:0] length,
  output reg[31:0] ip_accept,
  output reg dhcp_success,
  output reg dhcp_failed,

  //constants
  input [47:0] local_mac,
  output [47:0] dhcp_destination_mac,
  output [31:0] dhcp_destination_ip,
  output [15:0] dhcp_destination_port


  );

localparam TX_IDLE = 8'd0, DHCPDISCOVER = 8'd1, DHCPSEND = 8'd2, DHCPOFFER = 8'd3, DHCPREQUEST = 8'd4, DHCPRENEW = 8'd5;
localparam RX_IDLE = 8'd0, RX_OFFER_ACK = 8'd1, RX_DONE = 8'd2;

localparam DIS_TX_LEN = 16'd244;    // length of discover message
localparam REQ_TX_LEN = 16'd256;    // length of request message for boot (starting) state
localparam RENEW_TX_LEN = 16'd250;    // length of request message for lease renewal

reg [8:0] byte_no;
reg tx_request;
reg send_discovery;
reg is_renewal = 1'b0;    // Are we renewing the lease?
reg [8:0] state = TX_IDLE;

reg [47:0] destination_mac = 48'hFFFFFFFFFFFF;
reg [31:0] destination_ip = 32'hFFFFFFFF;
assign dhcp_destination_mac = destination_mac;
assign dhcp_destination_ip = destination_ip;

reg [15:0] xid_sum;    // random-ish number, constant after the first DHCP Discover
reg xid_done = 0;
wire [7:0] xid0, xid1, xid2, xid3;
assign xid0 = local_mac[15:8];  // this is the final transaction ID
assign xid1 = local_mac[7:0];
assign xid2 = xid_sum[15:8];
assign xid3 = xid_sum[7:0];

//---------------------------------------------------------------
//                DHCP Send
//---------------------------------------------------------------

always @ (posedge tx_clock)
begin
  case (state)
  TX_IDLE:
    begin
    if ( ! xid_done)
      xid_sum <= xid_sum + 1'd1;
    byte_no <= 9'b1;
    dhcp_tx_request <= 1'b0;
    send_discovery <= 1'b0;
      if (tx_enable) begin
        if (is_renewal)
          state <= DHCPRENEW;
        else
          state <= DHCPDISCOVER;
      end
      else if (rx_send_request) state <= DHCPREQUEST;
    end

  DHCPDISCOVER:
    begin
    xid_done <= 1'b1;
    length <= DIS_TX_LEN;
    dhcp_tx_request <= 1'b1;
      if (udp_tx_enable) begin
        tx_data <= 8'h01;          // tx_data needs to be available before udp_tx_active
        send_discovery <= 1'b1;      // hence this is byte_no 0
        state <= DHCPSEND;
      end
    end

  DHCPREQUEST:
    begin
    length <= REQ_TX_LEN;
    dhcp_tx_request <= 1'b1;
      if (udp_tx_enable) begin      // tx_data needs to be available before udp_tx_active
        tx_data <= 8'h01;          // hence this is byte_no 0
        state <= DHCPSEND;
      end
    end

  DHCPRENEW:
    begin
    length <= RENEW_TX_LEN;
    dhcp_tx_request <= 1'b1;
      if (udp_tx_enable) begin      // tx_data needs to be available before udp_tx_active
        tx_data <= 8'h01;          // hence this is byte_no 0
        state <= DHCPSEND;
      end
    end

  DHCPSEND:
    begin
      if (byte_no < length) begin
        if (udp_tx_active) begin
          case (byte_no)
          1: tx_data <= 8'h01;
          2: tx_data <= 8'h06;
          3: tx_data <= 8'h0;
          4: tx_data <= xid0;
          5: tx_data <= xid1;
          6: tx_data <= xid2;
          7: tx_data <= xid3;
          8: tx_data <= 8'd0;
          9: tx_data <= {4'd0, dhcp_seconds_timer};
          10: tx_data <= 8'd0;    // 2 zeros
          12: tx_data <= is_renewal ? ip_accept[31:24] : 8'd0;    // ciaddr
          13: tx_data <= is_renewal ? ip_accept[23:16] : 8'd0;
          14: tx_data <= is_renewal ? ip_accept[15:8]  : 8'd0;
          15: tx_data <= is_renewal ? ip_accept[7:0]   : 8'd0;
          16: tx_data <= 8'd0;    // 12 zeros
          28: tx_data <= local_mac[47:40];    // chaddr
          29: tx_data <= local_mac[39:32];
          30: tx_data <= local_mac[31:24];
          31: tx_data <= local_mac[23:16];
          32: tx_data <= local_mac[15:8];
          33: tx_data <= local_mac[7:0];
          34: tx_data <= 8'd0;    // 202  zeros
         236: tx_data <= 8'h63;
         237: tx_data <= 8'h82;
         238: tx_data <= 8'h53;
         239: tx_data <= 8'h63;
         240: tx_data <= 8'h35;
         241: tx_data <= 8'h01;
         242: tx_data <= send_discovery ? 8'h01 : 8'h03;
         243: tx_data <= send_discovery ? 8'hFF : 8'h32;  // send discovery ends here
         244: tx_data <= 8'h04;                // start of REQUEST
         245: tx_data <= ip_accept[31:24];    // Requested IP
         246: tx_data <= ip_accept[23:16];
         247: tx_data <= ip_accept[15:8];
         248: tx_data <= ip_accept[7:0];
         249: tx_data <= is_renewal ? 8'hFF : 8'h36;    // Renewal Request ends here
         250: tx_data <= 8'h04;    // Original Request requires the Server Identifier
         251: tx_data <= remote_ip[31:24];
         252: tx_data <= remote_ip[23:16];
         253: tx_data <= remote_ip[15:8];
         254: tx_data <= remote_ip[7:0];
         255: tx_data <= 8'hFF;
        endcase
        byte_no <= byte_no + 9'd1;
       end
       end
    else  state <= TX_IDLE;     // all data sent so return to start
     end
  endcase
end

//-----------------------------------------------------------------------------
//                               output
//-----------------------------------------------------------------------------
assign dhcp_destination_port = 16'd67;


//---------------------------------------------------------------
//                DHCP Receive
//---------------------------------------------------------------
reg [8:0]  rx_state;
reg [8:0]  rx_byte_no;
reg rx_send_request;        // in rx_clock domain
//reg [47:0] target_mac;
//reg [47:0] temp_target_mac;
reg [15:0] option;        // holds DHCP options
reg [7:0]  skip;          // number of DHCP option bytes to skip
reg [31:0] temp_ip_accept;



always @ (posedge rx_clock)
begin
 if (state == DHCPSEND) rx_send_request <= 1'b0;   // Only clear send request when tx has seen it

 if (dhcp_rx_active && rx_enable)
    case (rx_state)
      RX_IDLE:
      begin
      rx_byte_no <= 9'd1;
        if (rx_data == 8'h02) begin         // look for udp packet type of 0x02
            rx_state <= RX_OFFER_ACK;
        end
      end

    RX_OFFER_ACK:
      begin
        case (rx_byte_no)
              4: begin  // Accept only our own transaction ID
                  if (rx_data != xid0) rx_state <= RX_DONE;
                  rx_byte_no <=  rx_byte_no + 9'd1;
                end

              5: begin
                  if (rx_data != xid1) rx_state <= RX_DONE;
                  rx_byte_no <=  rx_byte_no + 9'd1;
                end

              6: begin
                  if (rx_data != xid2) rx_state <= RX_DONE;
                  rx_byte_no <=  rx_byte_no + 9'd1;
                end

              7: begin
                  if (rx_data != xid3) rx_state <= RX_DONE;
                  rx_byte_no <=  rx_byte_no + 9'd1;
                end

              // whilst this works, and takes less LEs, but the negative slack is worse.
              16,17,18,19: begin
                  temp_ip_accept <= {temp_ip_accept[23:0],rx_data};
                  rx_byte_no <=  rx_byte_no + 9'd1;
                end
              //16: begin
              //    temp_ip_accept[31:24] <= rx_data;      // get the offered IP address
              //    rx_byte_no <=  rx_byte_no + 9'd1;
              //   end

              //17: begin
              //    temp_ip_accept[23:16] <= rx_data;
              //    rx_byte_no <=  rx_byte_no + 9'd1;
              //   end

              //18: begin
              //    temp_ip_accept[15:8] <= rx_data;
              //    rx_byte_no <=  rx_byte_no + 9'd1;
              //   end

              //19: begin
              //    temp_ip_accept[7:0] <= rx_data;
              //    rx_byte_no <=  rx_byte_no + 9'd1;
              //   end
              //whilst this works, and takes less LEs, but the negative slack is worse.
                28,29,30,31,32,33: begin
                    if (rx_data != local_mac[271-8*rx_byte_no-:8]) rx_state <= RX_DONE;
                    else rx_byte_no <=  rx_byte_no + 9'd1;
                  end

                34: begin
                    ip_accept <= temp_ip_accept;              // for us so save the offered IP address
                    rx_byte_no <=  rx_byte_no + 9'd1;
                  end
                //compare target mac to our local_mac
              //28: begin
              //    target_mac[47:40] <= rx_data;        // get the target MAC address
              //    rx_byte_no <=  rx_byte_no + 9'd1;
              //   end
              //29: begin
              //    target_mac[39:32] <= rx_data;
              //    rx_byte_no <=  rx_byte_no + 9'd1;
              //   end
              //30: begin
              //    target_mac[31:24] <= rx_data;
              //    rx_byte_no <=  rx_byte_no + 9'd1;
              //   end
              //31: begin
              //    target_mac[23:16] <= rx_data;
              //    rx_byte_no <=  rx_byte_no + 9'd1;
              //   end
              //32: begin
              //    target_mac[15:8] <= rx_data;
              //    rx_byte_no <=  rx_byte_no + 9'd1;
              //   end
              //33: begin
              //    target_mac[7:0]  <= rx_data;
              //    rx_byte_no <=  rx_byte_no + 9'd1;
              //   end
              //34: begin
              //    if (local_mac != target_mac) rx_state <= RX_DONE;    // not this MAC address so
              //    else begin                              // wait for end of packet then re-start
              //          ip_accept  <= temp_ip_accept;              // for us so save the offered IP address
              //        rx_byte_no <=  rx_byte_no + 9'd1;
              //        end
              //   end

//          // read next two bytes and look for lease time, DHCP server IP address or end
              240: begin
                  option[15:8] <= rx_data;
                  if (rx_data == 8'hFF) begin // early exit if no padding in ACK
                    dhcp_success <= 1'b1;
                    dhcp_failed  <= 1'b0;
                    is_renewal <= 1'b1;
                    destination_mac <= remote_mac;  // save DHCP server addresses for renewal
                    destination_ip <= remote_ip;
                    rx_state <= RX_IDLE;
                  end else if (rx_data != 8'h00) begin // Handle pad
                    rx_byte_no <=  rx_byte_no + 9'd1;
                  end
                 end
              241: begin
                  option[7:0] <= rx_data;
                  rx_byte_no <=  rx_byte_no + 9'd1;
                 end
              242: begin
                  skip <= option[7:0];              // potentially skip these number of bytes
                  if (option == 16'h3501) begin // dhcp type since can be in any order
                    if(rx_data == 8'h02) begin          // if 0x02 then an offer so request it
                      rx_send_request <= 1'b1;
                      rx_state <= RX_DONE;            // wait until end of data then re-start
                    end
                    else if (rx_data == 8'h05) begin      // a DHCP ACK so read the options
                      rx_byte_no <=  240;
                    end
                    else begin                    // neither, so report a fail
                      dhcp_failed  <= 1'b1;
                      dhcp_success <= 1'b0;
                      rx_state <= RX_DONE;          // wait for end of packet then re-start
                    end
                  end
                  if (option == 16'h3304) begin           // get lease time
                    lease[31:24] <= rx_data;
                    rx_byte_no <= 243;
                  end
                  else if (option == 16'h3604) begin        // get DHCP sever IP address
                    server_ip[31:24] <= rx_data;
                    rx_byte_no <= 246;
                  end
                  else if (option[7:0] == 8'h01) begin // Handle size 1 option case
                    rx_byte_no <= 240;
                  end
                  else begin
                    rx_byte_no <= 249;
                    end
                  end

              243: begin
                  lease[23:16] <= rx_data;
                  rx_byte_no <=  rx_byte_no + 9'd1;
                 end

              244: begin
                  lease[15:8] <= rx_data;
                  rx_byte_no <=  rx_byte_no + 9'd1;
                 end

              245: begin
                  lease[7:0] <= rx_data;
                  rx_byte_no <= 240;
                  end

              246: begin
                  server_ip[23:16] <= rx_data;
                  rx_byte_no <=  rx_byte_no + 9'd1;
                 end

              247: begin
                  server_ip[15:8] <= rx_data;
                  rx_byte_no <=  rx_byte_no + 9'd1;
                 end

              248: begin
                  server_ip[7:0] <= rx_data;
                  rx_byte_no <= 240;
                end

              // we all ready have two 'skips' when we get here
              249: begin
                  if (skip - 2 == 0) rx_byte_no <= 240;      // skip over bytes then look again
                  else skip <= skip - 1'b1;
                end

          default: rx_byte_no <=  rx_byte_no + 9'd1;
        endcase

      end
    default: rx_state <= RX_IDLE;
  endcase

  else begin // !(dhcp_rx_active && rx_enable)
    if (!rx_enable) begin
      dhcp_success <= 1'b0;
      dhcp_failed <= 1'b0;
    end
    rx_state <= RX_IDLE;
  end
end

endmodule
