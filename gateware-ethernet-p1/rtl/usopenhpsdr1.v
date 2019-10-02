
// OpenHPSDR upstream (Card->PC) protocol packer

module usopenhpsdr1 (
  clk,
  have_ip,
  run,
  wide_spectrum,
  idhermeslite,
  mac,
  discovery,
  
  udp_tx_enable,
  udp_tx_request,
  udp_tx_data,
  udp_tx_length,

  bs_tdata,
  bs_tready,
  bs_tvalid,

  us_tdata,
  us_tlast,
  us_tready,
  us_tvalid,
  us_tuser,
  us_tlength,

  // Command slave interface
  cmd_addr,
  cmd_data,
  cmd_rqst,

  resp,
  resp_rqst,

  watchdog_up,

  usethasmi_send_more,
  usethasmi_erase_done,
  usethasmi_ack
);

input               clk;
input               have_ip;
input               run;
input               wide_spectrum;
input               idhermeslite;
input [47:0]        mac;
input               discovery;

input               udp_tx_enable;
output              udp_tx_request;
output [7:0]        udp_tx_data;
output logic [10:0] udp_tx_length = 'd0;

input [11:0]        bs_tdata;
output              bs_tready;
input               bs_tvalid;

input [23:0]        us_tdata;
input               us_tlast;
output              us_tready;
input               us_tvalid;
input [ 1:0]        us_tuser;
input [10:0]        us_tlength;

// Command slave interface
input  [5:0]        cmd_addr;
input  [31:0]       cmd_data;
input               cmd_rqst;

input  [39:0]       resp;
output logic        resp_rqst = 1'b0;

output logic        watchdog_up = 1'b0;

input               usethasmi_send_more;
input               usethasmi_erase_done;
output              usethasmi_ack;



parameter           NR = 8'h0;
parameter           HERMES_SERIALNO = 8'h0;


localparam START        = 4'h0,
           WIDE1        = 4'h1,
           WIDE2        = 4'h2,
           WIDE3        = 4'h3,
           WIDE4        = 4'h4,
           DISCOVER1    = 4'h5,
           DISCOVER2    = 4'h6,
           UDP1         = 4'h7,
           UDP2         = 4'h8,
           SYNC_RESP    = 4'h9,
           RXDATA2      = 4'ha,
           RXDATA1      = 4'hb,
           RXDATA0      = 4'hc,
           MIC1         = 4'hd,
           MIC0         = 4'he,
           PAD          = 4'hf;

logic   [ 3:0]  state = START;
logic   [ 3:0]  state_next;

logic   [10:0]  byte_no = 11'h00;
logic   [10:0]  byte_no_next; 

logic   [10:0]  udp_tx_length_next;

logic   [31:0]  ep6_seq_no = 32'h0;
logic   [31:0]  ep6_seq_no_next;

logic   [31:0]  ep4_seq_no = 32'h0;
logic   [31:0]  ep4_seq_no_next;

logic   [ 7:0]  discover_data = 'd0, discover_data_next;
logic   [ 7:0]  wide_data = 'd0, wide_data_next;
logic   [ 7:0]  udp_data = 'd0, udp_data_next;

// Allow for at least 12 receivers in a round of sample data
logic   [ 6:0]  round_bytes = 7'h00, round_bytes_next;

logic   [6:0]   bs_cnt = 7'h1, bs_cnt_next;
logic   [6:0]   set_bs_cnt = 7'h1;

logic           resp_rqst_next;

logic           watchdog_up_next;

logic           vna = 1'b0;

logic           vna_mic_bit = 1'b0, vna_mic_bit_next;

// Command Slave State Machine
always @(posedge clk) begin
  if (cmd_rqst) begin
    case (cmd_addr)
      6'h00: begin  
        // Shift no of receivers by speed
        set_bs_cnt <= ((cmd_data[7:3] + 1'b1) << cmd_data[25:24]);
      end

      6'h09: begin
        vna <= cmd_data[23];
      end
    endcase 
  end
end


// State
always @ (posedge clk) begin
  state <= state_next;

  byte_no <= byte_no_next;
  discover_data <= discover_data_next;
  wide_data <= wide_data_next;
  udp_data <= udp_data_next;

  udp_tx_length <= udp_tx_length_next;
  
  bs_cnt <= bs_cnt_next;

  round_bytes <= round_bytes_next;

  resp_rqst <= resp_rqst_next;

  watchdog_up <= watchdog_up_next;

  vna_mic_bit <= vna_mic_bit_next;

  if (~run) begin
    ep6_seq_no <= 32'h0;
    ep4_seq_no <= 32'h0;
  end else begin
    ep6_seq_no <= ep6_seq_no_next;
    // Synchronize sequence number lower 2 bits as some software may require this
    ep4_seq_no <= (bs_tvalid) ? ep4_seq_no_next : {ep4_seq_no_next[31:2],2'b00};
  end 
end 


// FSM Combinational
always @* begin

  // Next State
  state_next = state;

  byte_no_next = byte_no;
  discover_data_next = discover_data;
  wide_data_next = wide_data;
  udp_data_next = udp_data;

  udp_tx_length_next = udp_tx_length;

  round_bytes_next = round_bytes;

  ep6_seq_no_next = ep6_seq_no;
  ep4_seq_no_next = ep4_seq_no;

  bs_cnt_next = bs_cnt;

  resp_rqst_next = resp_rqst;

  watchdog_up_next = watchdog_up;

  vna_mic_bit_next = vna_mic_bit;

  // Combinational
  udp_tx_data = udp_data;
  udp_tx_request = 1'b0;
  us_tready = 1'b0;
  bs_tready = 1'b0;

  usethasmi_ack = 1'b0;

  case (state)
    START: begin
 
      if (discovery | usethasmi_erase_done | usethasmi_send_more) begin 
        udp_tx_length_next = 'h3c;
        state_next = DISCOVER1;

      end else if ((us_tlength > 11'd333) & us_tvalid & have_ip & run) begin // wait until there is enough data in fifo
        udp_tx_length_next = 'd1032;
        state_next = UDP1;
      
      end else if (bs_tvalid & ~(|bs_cnt)) begin 
        bs_cnt_next = set_bs_cnt; // Set count until next wide data
        watchdog_up_next = ~watchdog_up; 
        udp_tx_length_next = 'd1032;
        if (wide_spectrum) state_next = WIDE1;
      end
    end

    DISCOVER1: begin
      byte_no_next = 'h3a;
      udp_tx_data = discover_data;
      udp_tx_request = 1'b1;
      discover_data_next = 8'hef;
      if (udp_tx_enable) state_next = DISCOVER2;
    end // DISCOVER1:

    DISCOVER2: begin
      byte_no_next = byte_no - 11'd1;
      udp_tx_data = discover_data;      
      case (byte_no[5:0])
        6'h3a: discover_data_next = 8'hfe;
        6'h39: discover_data_next = usethasmi_erase_done ? 8'h03 : (usethasmi_send_more ? 8'h04 : (run ? 8'h03 : 8'h02));
        6'h38: discover_data_next = mac[47:40];
        6'h37: discover_data_next = mac[39:32];
        6'h36: discover_data_next = mac[31:24];
        6'h35: discover_data_next = mac[23:16];
        6'h34: discover_data_next = mac[15:8];
        6'h33: discover_data_next = mac[7:0];
        6'h32: discover_data_next = HERMES_SERIALNO;
        //6'h31: discover_data_next = IDHermesLite ? 8'h06 : 8'h01;
        // FIXME: Really needed for CW skimmer? Why so much?
        6'h30: discover_data_next = "H";
        6'h2f: discover_data_next = "E";
        6'h2e: discover_data_next = "R";
        6'h2d: discover_data_next = "M";
        6'h2c: discover_data_next = "E";
        6'h2b: discover_data_next = "S";
        6'h2a: discover_data_next = "L";
        6'h29: discover_data_next = "T";
        6'h28: discover_data_next = NR[7:0];
        6'h00: begin
          discover_data_next = idhermeslite ? 8'h06 : 8'h01;
          if (usethasmi_erase_done | usethasmi_send_more) byte_no_next = 6'h00;
          else state_next = START;
        end
        default: begin
          discover_data_next = idhermeslite ? 8'h06 : 8'h01;
        end
      endcase

      // Always acknowledge
      usethasmi_ack = byte_no[5:0] <= 6'h38;
    end

    // start sending UDP/IP data
    WIDE1: begin
      byte_no_next = 'h406;
      udp_tx_data = wide_data;
      udp_tx_request = 1'b1;
      wide_data_next = 8'hef;
      if (udp_tx_enable) state_next = WIDE2;
    end 

    WIDE2: begin
      byte_no_next = byte_no - 11'd1;
      udp_tx_data = wide_data;
      case (byte_no[2:0])
        3'h6: wide_data_next = 8'hfe;
        3'h5: wide_data_next = 8'h01;
        3'h4: wide_data_next = 8'h04;
        3'h3: wide_data_next = ep4_seq_no[31:24];
        3'h2: wide_data_next = ep4_seq_no[23:16];
        3'h1: wide_data_next = ep4_seq_no[15:8];
        3'h0: begin
          wide_data_next = ep4_seq_no[7:0];
          ep4_seq_no_next = ep4_seq_no + 'h1;
          state_next = WIDE3;
        end
        default: wide_data_next = 8'hxx;
      endcase
    end

    WIDE3: begin
      byte_no_next = byte_no - 11'd1;
      udp_tx_data = wide_data;
      //wide_data_next = {{4{bs_tdata[11]}}, bs_tdata};
      wide_data_next = bs_tdata[7:0];

      // Allow for one extra to keep udp_tx_data mux stable
      state_next = (&byte_no) ? START : WIDE4;
    end 

    WIDE4: begin
      byte_no_next = byte_no - 11'd1;
      udp_tx_data = wide_data;
      //wide_data_next = bs_tdata[7:0];
      wide_data_next = {{4{bs_tdata[11]}}, bs_tdata[11:8]};
      bs_tready = 1'b1; // Pop data

      // Escape if something goes wrong
      state_next = (&byte_no) ? START : WIDE3;
    end

    UDP1: begin
      byte_no_next = 'h406;
      udp_tx_request = 1'b1;
      udp_data_next = 8'hef;
      if (udp_tx_enable) state_next = UDP2;
    end
    
    UDP2: begin
      byte_no_next = byte_no - 11'd1;
      case (byte_no[2:0])
        3'h6: udp_data_next = 8'hfe;
        3'h5: udp_data_next = 8'h01;
        3'h4: udp_data_next = 8'h06;
        3'h3: udp_data_next = ep6_seq_no[31:24];
        3'h2: udp_data_next = ep6_seq_no[23:16];
        3'h1: udp_data_next = ep6_seq_no[15:8]; 
        3'h0: begin
          udp_data_next = ep6_seq_no[7:0];
          ep6_seq_no_next = ep6_seq_no + 'h1;
          bs_cnt_next = bs_cnt - 7'd1;
          state_next = SYNC_RESP;
        end
        default: udp_data_next = 8'hxx;
      endcase // byte_no
    end // UDP2:

    SYNC_RESP: begin
      byte_no_next = byte_no - 11'd1;
      round_bytes_next = 'd0;
      case (byte_no[8:0])
        9'h1ff: udp_data_next = 8'h7f;
        9'h1fe: udp_data_next = 8'h7f;
        9'h1fd: udp_data_next = 8'h7f;
        9'h1fc: udp_data_next = resp[39:32];
        9'h1fb: udp_data_next = resp[31:24];
        9'h1fa: udp_data_next = resp[23:16];
        9'h1f9: udp_data_next = resp[15:8];
        9'h1f8: begin 
          udp_data_next = resp[7:0];
          resp_rqst_next = ~resp_rqst; 
          state_next = RXDATA2; 
        end 
        default: udp_data_next = 8'hxx; 
      endcase
    end

    RXDATA2: begin
      byte_no_next = byte_no - 11'd1;
      round_bytes_next = round_bytes + 7'd1;
      udp_data_next = us_tdata[23:16];
      vna_mic_bit_next = us_tuser[0]; // Save mic bit for use laster with mic data

      if (|byte_no[8:0]) begin
        state_next = RXDATA1;
      end else begin 
        state_next = byte_no[9] ? SYNC_RESP : START;
      end
    end      

    RXDATA1: begin
      byte_no_next = byte_no - 11'd1;
      round_bytes_next = round_bytes + 7'd1;
      udp_data_next = us_tdata[15:8];

      if (|byte_no[8:0]) begin
        state_next = RXDATA0;        
      end else begin
        state_next = byte_no[9] ? SYNC_RESP : START; 
      end
    end   

    RXDATA0: begin
      byte_no_next = byte_no - 11'd1;
      round_bytes_next = round_bytes + 7'd1;
      udp_data_next = us_tdata[7:0];
      us_tready = 1'b1; // Pop next word

      if (|byte_no[8:0]) begin
        if (us_tlast) begin 
          state_next = MIC1;
        end else begin
          state_next = RXDATA2;
        end
      end else begin 
        state_next = byte_no[9] ? SYNC_RESP : START;
      end
    end 

    MIC1: begin
      byte_no_next = byte_no - 11'd1;
      round_bytes_next = round_bytes + 7'd1;
      udp_data_next = 'd0;
      
      if (|byte_no[8:0]) begin
        state_next = MIC0;
      end else begin 
        state_next = byte_no[9] ? SYNC_RESP : START;
      end   
    end 

    MIC0: begin
      byte_no_next = byte_no - 11'd1;
      round_bytes_next = 'd0;
      udp_data_next = vna ? {7'h00,vna_mic_bit} : 8'h00; // VNA, may need to be in MIC1

      if (|byte_no[8:0]) begin
        // Enough room for another round of data?
        state_next = (byte_no[8:0] > round_bytes) ? RXDATA2 : PAD;
      end else begin
        state_next = byte_no[9] ? SYNC_RESP : START;
      end
    end

    PAD: begin
      byte_no_next = byte_no - 11'd1;
      udp_data_next = 8'h00;

      if (~(|byte_no[8:0])) begin
        state_next = byte_no[9] ? SYNC_RESP : START;
      end 
    end 

    default: state_next = START;

  endcase // state
end // always @*


endmodule
