/*

--------------------------------------------------------------------------------
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
Boston, MA  02110-1301, USA.
--------------------------------------------------------------------------------


---------------------------------------------------------------------------------
  Copywrite (C) Phil Harman VK6PH May 2014
---------------------------------------------------------------------------------
  
  The code implements an Iambic CW keyer.  The following features are supported:
    
    * Variable speed control from 1 to 60 WPM
    * Dot and Dash memory
    * Straight, Bug, Iambic Mode A or B Modes
    * Variable character weighting
    * Automatic Letter spacing
    * Paddle swap
    
  Dot and Dash memory works by registering an alternative paddle closure whilst a paddle is pressed.
  The alternate paddle closure can occur at any time during a paddle closure and is not limited to being 
  half way through the current dot or dash. This feature could be added if required.
  
  In Straight mode, closing the DASH paddle will result in the output following the input state.  This enables a 
  straight morse key or external Iambic keyer to be connected.
  
  In Bug mode closing the dot paddle will send repeated dots.
  
  The CWX input simply cases the output to follow the input and is intended for use with PC keyboard to CW programs.
  
  The difference between Iambic Mode A and B lies in what the keyer does when both paddles are released. In Mode A the 
  keyer completes the element being sent when both paddles are released. In Mode B the keyer sends an additional
  element opposite to the one being sent when the paddles are released. 
  
  Automatic Letter Space works as follows: When enabled, if you pause for more than one dot time between a dot or dash
  the keyer will interpret this as a letter-space and will not send the next dot or dash until the letter-space time has been met.
  The normal letter-space is 3 dot periods. The keyer has a paddle event memory so that you can enter dots or dashes during the
  inter-letter space and the keyer will send them as they were entered. 

  Speed calculation -  Using standard PARIS timing, dot_period(mS) = 1200/WPM
  
  Changes:
  
  2014 May 8  - First release
       N0v 14 - Added individual inputs for iambic and keyer_mode

  2019 July 21 - Optimized to reduce resources and work with Hermes-Lite 2.0 kf7o


*/

module iambic (
  input clock,              // 2.5 MHz       
  input [5:0] cw_speed,     // 1 to 60 WPM
  input iambic,             // 0 = straight/bug,  1 = Iambic 
  input keyer_mode,         // 0 = Mode A, 1 = Mode B
  input [7:0] weight,       // 33 to 66, nominal is 50
  input letter_space,       // 0 = off, 1 = on
  input dot_key,            // dot paddle  input, active high
  input dash_key,           // dash paddle input, active high
  input CWX,                // CW data from PC active high
  input paddle_swap,        // swap if set
  output reg keyer_out,     // keyer output, active high
  input IO5                 // additional CW key via digital input IO5, debounced, inverted
);


logic [9:0]   cw_speed_dot17;


// Cheap divide 6 bits -> 10 bits
// cw_speed_dot17 is the dot time in units of 2.5 MHz clock divided by 64
// cw_speed of 5wpm is (551 * 64 * 17)/2.5e6 = 239.752ms
// This is close to 1200/5 = 240ms
// The slowest speed is a bit more than 2wpm, 0-2wpm are rounded to this
// div = 64
// dt = 1.0/2.5e6
// for i in range(1,64): 
//   ms = 1200.0/i/1000
//   units = int(round((ms/17.0)*2.5e6/div))
//   approximate = units * 17 * div * dt
//   error = 100*abs(approximate-ms)/ms
//   print("{0} : cw_speed_dot17 = {1}; // real:{2:.2f}ms approximate:{3:.2f}ms error:{4:.2f}%".format(i,units,1000*ms,1000*approximate,error))

always @* begin
  case (cw_speed) 
    0  : cw_speed_dot17 = 1023;
    1  : cw_speed_dot17 = 1023;
    2  : cw_speed_dot17 = 1023;
    3  : cw_speed_dot17 = 919; // real:400.00ms approximate:399.95ms error:0.01%
    4  : cw_speed_dot17 = 689; // real:300.00ms approximate:299.85ms error:0.05%
    5  : cw_speed_dot17 = 551; // real:240.00ms approximate:239.80ms error:0.09%
    6  : cw_speed_dot17 = 460; // real:200.00ms approximate:200.19ms error:0.10%
    7  : cw_speed_dot17 = 394; // real:171.43ms approximate:171.47ms error:0.02%
    8  : cw_speed_dot17 = 345; // real:150.00ms approximate:150.14ms error:0.10%
    9  : cw_speed_dot17 = 306; // real:133.33ms approximate:133.17ms error:0.12%
    10 : cw_speed_dot17 = 276; // real:120.00ms approximate:120.12ms error:0.10%
    11 : cw_speed_dot17 = 251; // real:109.09ms approximate:109.24ms error:0.13%
    12 : cw_speed_dot17 = 230; // real:100.00ms approximate:100.10ms error:0.10%
    13 : cw_speed_dot17 = 212; // real:92.31ms approximate:92.26ms error:0.05%
    14 : cw_speed_dot17 = 197; // real:85.71ms approximate:85.73ms error:0.02%
    15 : cw_speed_dot17 = 184; // real:80.00ms approximate:80.08ms error:0.10%
    16 : cw_speed_dot17 = 172; // real:75.00ms approximate:74.85ms error:0.19%
    17 : cw_speed_dot17 = 162; // real:70.59ms approximate:70.50ms error:0.12%
    18 : cw_speed_dot17 = 153; // real:66.67ms approximate:66.59ms error:0.12%
    19 : cw_speed_dot17 = 145; // real:63.16ms approximate:63.10ms error:0.09%
    20 : cw_speed_dot17 = 138; // real:60.00ms approximate:60.06ms error:0.10%
    21 : cw_speed_dot17 = 131; // real:57.14ms approximate:57.01ms error:0.23%
    22 : cw_speed_dot17 = 125; // real:54.55ms approximate:54.40ms error:0.27%
    23 : cw_speed_dot17 = 120; // real:52.17ms approximate:52.22ms error:0.10%
    24 : cw_speed_dot17 = 115; // real:50.00ms approximate:50.05ms error:0.10%
    25 : cw_speed_dot17 = 110; // real:48.00ms approximate:47.87ms error:0.27%
    26 : cw_speed_dot17 = 106; // real:46.15ms approximate:46.13ms error:0.05%
    27 : cw_speed_dot17 = 102; // real:44.44ms approximate:44.39ms error:0.12%
    28 : cw_speed_dot17 = 98; // real:42.86ms approximate:42.65ms error:0.48%
    29 : cw_speed_dot17 = 95; // real:41.38ms approximate:41.34ms error:0.09%
    30 : cw_speed_dot17 = 92; // real:40.00ms approximate:40.04ms error:0.10%
    31 : cw_speed_dot17 = 89; // real:38.71ms approximate:38.73ms error:0.06%
    32 : cw_speed_dot17 = 86; // real:37.50ms approximate:37.43ms error:0.19%
    33 : cw_speed_dot17 = 84; // real:36.36ms approximate:36.56ms error:0.53%
    34 : cw_speed_dot17 = 81; // real:35.29ms approximate:35.25ms error:0.12%
    35 : cw_speed_dot17 = 79; // real:34.29ms approximate:34.38ms error:0.28%
    36 : cw_speed_dot17 = 77; // real:33.33ms approximate:33.51ms error:0.53%
    37 : cw_speed_dot17 = 75; // real:32.43ms approximate:32.64ms error:0.64%
    38 : cw_speed_dot17 = 73; // real:31.58ms approximate:31.77ms error:0.60%
    39 : cw_speed_dot17 = 71; // real:30.77ms approximate:30.90ms error:0.42%
    40 : cw_speed_dot17 = 69; // real:30.00ms approximate:30.03ms error:0.10%
    41 : cw_speed_dot17 = 67; // real:29.27ms approximate:29.16ms error:0.38%
    42 : cw_speed_dot17 = 66; // real:28.57ms approximate:28.72ms error:0.53%
    43 : cw_speed_dot17 = 64; // real:27.91ms approximate:27.85ms error:0.19%
    44 : cw_speed_dot17 = 63; // real:27.27ms approximate:27.42ms error:0.53%
    45 : cw_speed_dot17 = 61; // real:26.67ms approximate:26.55ms error:0.45%
    46 : cw_speed_dot17 = 60; // real:26.09ms approximate:26.11ms error:0.10%
    47 : cw_speed_dot17 = 59; // real:25.53ms approximate:25.68ms error:0.57%
    48 : cw_speed_dot17 = 57; // real:25.00ms approximate:24.81ms error:0.77%
    49 : cw_speed_dot17 = 56; // real:24.49ms approximate:24.37ms error:0.48%
    50 : cw_speed_dot17 = 55; // real:24.00ms approximate:23.94ms error:0.27%
    51 : cw_speed_dot17 = 54; // real:23.53ms approximate:23.50ms error:0.12%
    52 : cw_speed_dot17 = 53; // real:23.08ms approximate:23.07ms error:0.05%
    53 : cw_speed_dot17 = 52; // real:22.64ms approximate:22.63ms error:0.05%
    54 : cw_speed_dot17 = 51; // real:22.22ms approximate:22.20ms error:0.12%
    55 : cw_speed_dot17 = 50; // real:21.82ms approximate:21.76ms error:0.27%
    56 : cw_speed_dot17 = 49; // real:21.43ms approximate:21.32ms error:0.48%
    57 : cw_speed_dot17 = 48; // real:21.05ms approximate:20.89ms error:0.77%
    58 : cw_speed_dot17 = 48; // real:20.69ms approximate:20.89ms error:0.97%
    59 : cw_speed_dot17 = 47; // real:20.34ms approximate:20.45ms error:0.57%
    60 : cw_speed_dot17 = 46; // real:20.00ms approximate:20.02ms error:0.10%
    61 : cw_speed_dot17 = 45; // real:19.67ms approximate:19.58ms error:0.45%
    62 : cw_speed_dot17 = 44; // real:19.35ms approximate:19.15ms error:1.06%
    63 : cw_speed_dot17 = 44; // real:19.05ms approximate:19.15ms error:0.53%
  endcase // cw_speed
end

// Counter that counts 1/17 the time of a dot .2176/17
// This is to match the weight offset
// A weight of 51 is then 51/17= 3 or 3x a dot time
always @(posedge clock) begin
  if (dot_cnt_rst || dot_cnt == 0) begin
    dot_cnt <= (cw_speed_dot17 << 6);
  end else begin
    dot_cnt <= dot_cnt - 1;
  end
end

assign dot_done = (dot_cnt == 0);

        
                        
localparam  LOOP        = 0,
            PREDOT      = 1,
            PREDASH     = 2,
            SENDDOT     = 3,
            SENDDASH    = 4,
            DOTDELAY    = 5,
            DASHDELAY   = 6,
            DOTHELD     = 7,
            DASHHELD    = 8,
            LETTERSPACE = 9;  
   
        
reg dot_memory = 0;
reg dash_memory = 0;  
reg [4:0] key_state = 0;  

reg  [DELAYDASH-1:0] delay;
wire [DELAYDOT-1:0]  dot_delay;
wire [DELAYDASH-1:0] dash_delay;


// swap paddles if set
wire dot, dash;
assign dot  = paddle_swap ? dash_key : dot_key;
assign dash = paddle_swap ? dot_key  : dash_key;
    




always @* begin

  keyer_out   = 1'b0;
  delay_reset = 1'b0;


  if (dot_done) delay_next = delay + 1;
  else delay_next = delay;

  case (key_state)
    // wait for key press
    LOOP: begin
      dot_memory_next  = 1'b0;
      dash_memory_next = 1'b0;
      delay_reset = 1'b1;
      if(!iambic) begin           // Straight/External key or bug
        if (dash) keyer_out = 1'b1;
        else if (dot) key_state_next = SENDDOT;
        else keyer_out = CWX;   // neither so use CWX
      
      end else begin
        if (dot) key_state_next = SENDDOT;
        else if (dash) key_state_next = SENDDASH;
        else keyer_out = (CWX | IO5);  // neither so use CWX or IO5 ext CW digital input
      end   
    end 
    
    SENDDOT: begin 
      keyer_out = 1'b1;
      if (delay == 8'd17) begin
        delay_reset = 1'b1;
        key_state_next = DOTDELAY;        // add inter-character spacing of one dot length
      end
        
      // if Mode A and both paddles are released then clear dash memory
      if (keyer_mode == 0) begin  
        if (!dot & !dash) dash_memory_next = 1'b0;
      end else if (dash) begin            // set dash memory
        dash_memory_next = 1'b1;
      end
    end

    SENDDASH: begin
      keyer_out = 1'b1;
      if (delay == weight) begin
        delay_reset = 1'b1;
        key_state_next = DASHDELAY;       // add inter-character spacing of one dot length
      end
        
      // if Mode A and both padles are relesed then clear dot memory
      if (keyer_mode == 0) begin  
        if (!dot & !dash) dot_memory_next = 0;
      end else if (dot) begin               // set dot memory  
        dot_memory_next = 1'b1;
      end
    end
    
    // add dot delay at end of the dot and check for dash memory, then check if paddle still held
    DOTDELAY: begin
      if (delay == 8'd17) begin

        delay_reset = 1'b1;
        dot_memory_next  = 1'b0;
        dash_memory_next = 1'b0;
        key_state_next = LOOP;

        if(!iambic) begin                   // just return if in bug mode
          ;
        end else if (dash_memory | dash) begin             // dash has been set during the dot so service
          key_state_next = SENDDASH;
        end else if (dot) begin
          key_state_next = SENDDOT;
        end else if (letter_space) begin        // Letter space enabled so clear any pending dots or dashes 
          key_state_next = LETTERSPACE;
        end
      end else if (dash) begin
        dash_memory_next = 1'b1;
      end
    end
        
    // add dot delay at end of the dash and check for dot memory, then check if paddle still held
    DASHDELAY: begin
      if (delay == 8'd17) begin

        delay_reset = 1'b1;
        dot_memory_next  = 1'b0;
        dash_memory_next = 1'b0;
        key_state_next = LOOP;

        if(!iambic) begin                  
          ;
        end else if (dot_memory | dot) begin             
          key_state_next = SENDDOT;
        end else if (dash) begin
          key_state_next = SENDDASH;
        end else if (letter_space) begin      
          key_state_next = LETTERSPACE;
        end
      end else if (dot) begin
        dot_memory_next = 1'b1;
      end
    end
    
    // Add letter space (2 x dot delay) to end of character and check if a paddle is pressed during this time. 
    LETTERSPACE: begin
      if (delay == 8'd34) begin

        delay_reset = 1'b1;
        dot_memory_next  = 1'b0;
        dash_memory_next = 1'b0;
        key_state_next = LOOP;

        if (dot_memory) key_state_next = SENDDOT;
        else if (dash_memory) key_state_next = SENDDASH;
      
      end else begin       
        // save any key presses during the letter space delay
        if (dot)   dot_memory_next = 1'b1;
        if (dash) dash_memory_next = 1'b1;
      end
    end  
    
    default: key_state_next = LOOP;
  endcase
end





endmodule
