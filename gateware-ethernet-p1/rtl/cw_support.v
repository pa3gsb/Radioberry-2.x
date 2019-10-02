

module cw_support(
  clk,
  millisec_pulse,
  cw_hang_time,
  dot_key,
  dash_key,
  dot_key_debounced,
  dash_key_debounced,
  cw_power_on,
  cw_keydown
);

input         clk;
input         millisec_pulse;

input [9:0]   cw_hang_time;

input         dot_key;
input         dash_key;

input         dot_key_debounced;
input         dash_key_debounced;

output logic  cw_power_on = 1'b0;
output        cw_keydown;


// TX sequence logic. Delay CW envelope until T/R relay switches and the amp power turns on.
logic [16:0] cw_delay_line = 17'b0; // Delay CW press/release one mSec per unit. There is additional delay from debounce ext_cwkey.
logic [9:0]  cw_power_timeout = 10'b0;  // Keep power on after the key goes up. Delay is one mSec per count starting from key down.
logic        cw_count_state;    // State 0: first count for KEY_UP_TIMEOUT; State 1: second count for cw_hang_time.
localparam KEY_UP_TIMEOUT = 10'd41; // Minimum timeout. Must be the delay line time plus waveform decay time plus ending time.


always @(posedge clk)       // Delay the CW key press and release while preserving the timing.
  if (millisec_pulse)
    cw_delay_line <= {cw_delay_line[15:0], dot_key_debounced};
assign cw_keydown = cw_delay_line[16];

always @(posedge clk) begin   // Turn on CW power and T/R relay at first key press and hold for the delay time.
  if (dot_key) begin      // Start timing when the key first goes down.
    cw_power_timeout <= KEY_UP_TIMEOUT;
    cw_count_state <= 1'b0;
    cw_power_on <= 1'b1;
  end else if (millisec_pulse) begin  // Check every millisecond
    if (cw_power_timeout != 0) begin
      cw_power_timeout <= cw_power_timeout - 1'b1;
    end else if (cw_count_state == 1'b0) begin  // First count for KEY_UP_TIMEOUT, the minimum count.
      cw_power_timeout <= cw_hang_time;
      cw_count_state <= 1'b1;
    end else begin    // Second count for extra time cw_hang_time.
      cw_power_on <= 1'b0;
    end
  end
end

endmodule