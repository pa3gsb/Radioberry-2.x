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
	
	The difference between Iambic Mode A and B lies in what the keyer does when both paddles are released. In Mode A the 
	keyer completes the element being sent when both paddles are released. In Mode B the keyer sends an additional
	element opposite to the one being sent when the paddles are released. 
	
	This only effects letters and characters like C, period or AR. The following diagram shows the difference between 
	sending the character C in each mode.
	
	Mode A
	
					------													--------------
	Dash Paddle   	  |												   |				
						  -------------------------------------------------
					  
					  
	Dot Paddle ----------                                              --------------
							  |											   |
				            ---------------------------------------------
				         
	Keyer Output 	     -------------      ----      -------------      ----
						 |            |    |    |    |             |    |    |
					-----              ----      ----               ----      ------------
				         
				         
	Mode B
	
					------										--------------
	Dash Paddle   	      |									   |				
						  -------------------------------------
					  
					  
	Dot Paddle 		----------                                  --------------
							  |								   |
				              ---------------------------------
				         
	Keyer Output 	     -------------      ----      -------------      ----
						 |            |    |    |    |             |    |    |
					-----              ----      ----               ----      ------------			         
	
	
	Automatic Letter Space works as follows: When enabled, if you pause for more than one dot time between a dot or dash
	the keyer will interpret this as a letter-space and will not send the next dot or dash until the letter-space time has been met.
	The normal letter-space is 3 dot periods. The keyer has a paddle event memory so that you can enter dots or dashes during the
	inter-letter space and the keyer will send them as they were entered. 

	Speed calculation -  Using standard PARIS timing, dot_period(mS) = 1200/WPM
	
	Changes:
	
	2014 May 8 	- First release


*/

module iambic (
				input clock,					
				input [5:0] cw_speed,			// 1 to 60 WPM
				input [1:0] iambic_mode,		// 00 = straight/bug, 01 = Mode A, 10 = Mode B
				input [7:0] weight, 				// 33 to 66, nominal is 50
				input letter_space,				// 0 = off, 1 = on
				input dot_key,						// dot paddle input, active high
				input dash_key,					// dash paddle input, active high
				input CWX,						   // CW data from PC
				input paddle_swap,				// swap if set
				output reg keyer_out				// keyer output, active high
				);
				
parameter   clock_speed = 30;					// default clock speed of 30kHz from PLL 
														// overwrite using clock speed in kHz. Don't use < 10kHz
												
localparam 	LOOP 		= 0,
			PREDOT      = 1,
			PREDASH     = 2,
			SENDDOT 		= 3,
			SENDDASH		= 4,
			DOTDELAY 	= 5,
			DASHDELAY 	= 6,
			DOTHELD 		= 7,
			DASHHELD 	= 8,
			LETTERSPACE = 9;	
			
localparam  DELAYDOT 	= clogb2(1200 * clock_speed);  	          // worse case number of bits needed to hold dot delay counter
localparam  DELAYDASH 	= clogb2(1200 * clock_speed * 3 * 66/50); // worse case number of bits needed for dash delay counter	

				
reg dot_memory = 0;
reg dash_memory = 0;	
reg [4:0] key_state = 0;	

reg  [DELAYDASH-1:0] delay;
wire [DELAYDOT-1:0]  dot_delay;
wire [DELAYDASH-1:0] dash_delay;


assign  dot_delay  = (1200 * clock_speed)/cw_speed;	
assign  dash_delay = (dot_delay * 3 * weight)/50; 	// will be 3 * dot length at standard weight

// swap paddles if set
wire dot, dash;
assign dot 	= paddle_swap ? dash_key : dot_key;
assign dash = paddle_swap ? dot_key  : dash_key;
		

always @ (posedge clock)
begin
case (key_state)

// wait for key press
LOOP:
	begin
		 if(iambic_mode == 2'b00) begin			// Straight/External key or bug
			if (dash)									// send manual dashes
				keyer_out <= 1'b1;
			else if (dot)								// and automatic dots
				key_state <= PREDOT;
			else keyer_out <= CWX;	
		end
		else begin
			if (dot) 
				key_state <= PREDOT;
			else if (dash)
				key_state <= PREDASH;
			else keyer_out <= CWX;
		end 	
	end 
		
PREDOT:						// need to clear any pending dots or dashes 
	begin 
		clear_memory;
		key_state <= SENDDOT;
	end 
	
PREDASH:
	begin
		clear_memory;
		key_state <= SENDDASH;
	end 

// dot paddle  pressed so set keyer_out high for time dependant on speed
// also check if dash paddle is pressed during this time
SENDDOT:
   	begin 
	keyer_out <= 1'b1;
		if (delay == dot_delay)begin
			delay <= 0;
			keyer_out <= 1'b0;
			key_state <= DOTDELAY;				// add inter-character spacing of one dot length
		end
		else delay <= delay  + 1;
		
	// if Mode A and both paddels are relesed then clear dash memory
	if (iambic_mode == 2'b01) begin	
		if (!dot & !dash)
				dash_memory <= 0;
	end	
	else if (dash)							// set dash memory
		dash_memory <= 1'b1;
   end
		
// dash paddle pressed so set keyer_out high for time dependant on 3 x dot delay and weight
// also check if dot paddle is pressed during this time 
SENDDASH:
	begin
	keyer_out <= 1'b1;
		if (delay == dash_delay) begin
			delay <= 0;
			keyer_out <= 1'b0;
			key_state <= DASHDELAY;				// add inter-character spacing of one dot length
		end
		else delay <= delay  + 1;
		
	// if Mode A and both padles are relesed then clear dot memory
	if (iambic_mode == 2'b01 ) begin	
		if (!dot & !dash)
				dot_memory <= 0;
	end
	else if (dot)								// set dot memory  
		dot_memory <= 1'b1;
	end

// add dot delay at end of the dot and check for dash memory, then check if paddle still held
DOTDELAY:
	begin
		if (delay == dot_delay) begin
			delay <= 0;
			if(iambic_mode == 2'b00) 					// just return if in bug mode
				key_state <= LOOP;
			else if (dash_memory) 					// dash has been set during the dot so service
				key_state <= PREDASH;
			else key_state <= DOTHELD;				// dot is still active so service
		end
		else delay <= delay  + 1;
	if (dash)								// set dash memory
		dash_memory <= 1'b1;
	end
		
// add dot delay at end of the dash and check for dot memory, then check if paddle still held
DASHDELAY:
	begin
		if (delay == dot_delay) begin
			delay <= 0;
			if (dot_memory)						// dot has been set during the dash so service
				key_state <= PREDOT;
			else key_state <= DASHHELD;			// dash is still active so service
		end
		else delay <= delay  + 1;
		if (dot)							// set dot memory 
			dot_memory <= 1'b1;
	end
				
// check if dot paddle is still held, if so repeat the dot. Else check if Letter space is required	
DOTHELD:
	begin
		if (dot) 						// dot has been set during the dash so service
			key_state <= PREDOT;
		else if (dash)  				// has dash paddle been pressed
			key_state <= PREDASH;
		else if (letter_space) begin		// Letter space enabled so clear any pending dots or dashes 
			clear_memory;
			key_state <= LETTERSPACE;
		end 
		else key_state <= LOOP;
	end

// check if dash paddle is still held, if so repeat the dash. Else check if Letter space is required		
DASHHELD:
	begin
		if (dash) 						// dash has been set during the dot so service
			key_state <= PREDASH;
		else if (dot) 					// has dot paddle been pressed
			key_state <= PREDOT;
		else if (letter_space) begin		// Letter space enabled so clear any pending dots or dashes
			clear_memory;
			key_state <= LETTERSPACE;
		end
		else key_state <= LOOP;
	end 

// Add letter space (3 x dot delay) to end of character and check if a paddle is pressed during this time. 
// Actually add 2 x dot_delay since we already have a dot delay at the end of the character.
LETTERSPACE:
	begin
		if (delay == 2 * dot_delay) begin	
			delay <= 0;
			if (dot_memory) 			// check if a dot or dash paddle was pressed during the delay.
				key_state <= PREDOT;
			else
			if (dash_memory) 
				key_state <= PREDASH;
			else key_state <= LOOP;		// no memories set so restart
		end
		else delay <= delay + 1;
		
		// save any key presses during the letter space delay
		if (dot)   dot_memory <= 1'b1;
		if (dash) dash_memory <= 1'b1;
	end  

default: key_state <= 0;
endcase
end

task clear_memory;
begin 
	dot_memory  <= 0;
	dash_memory <= 0;
end
endtask	

function integer clogb2;
input [31:0] depth;
begin
  for(clogb2 = 0; depth > 0; clogb2 = clogb2 + 1)
  depth = depth >> 1;
end
endfunction



endmodule
