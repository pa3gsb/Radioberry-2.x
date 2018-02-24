//
//  HPSDR - High Performance Software Defined Radio
//
//  Hermes code. 
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


//  Profile - 2014  (C) Phil Harman VK6PH


// Raised cosine will be applied over 5mS. Profile consists of 1000 samples and is read from ROM.
// ROM address is read forwards for key press and backwards for key release.
// Clock is at 192kHz which is an integer multiple of 48kHz so we don't cross clock domains. 
// profile_ROM holds the profile values. Values are in profile.mif.
// For reverse read we need to keep the CW note and carrier operating - use PTT to do this.
// The max value of the profile has been slected to give the maximum output of the RF DAC taking into account
// the CORDIC gain. 
// When used to profile the RF a delay can be set that delays the RF following the key being pressed. 
// The delay is applied to the CW element such that length of the element is maintained i.e. the length
// of the element is increased by the delay time. 
// IMPORTANT:  The lenght of the delay must be shorter than the duration of a dot.
// At the end of the trailing profile a hang delay can be applied after no key closures for the hang delay period. 
// This extends the PTT for the delay period. During the hang delay the key is monitored.

// NOTE:  At 20 WPM a dot is 60mS and at 60 WPM 20mS using standard PARIS timing


module profile (
	input clock,								// 192kHz clock from PLL
	input CW_char,								// active when dot or dash key pressed
	input [7:0]delay,							// delay creation of profile for these mS
	input [9:0]hang,							// keep PTT active at end of character for these mS
	output [15:0]profile,			      // profile out for sidetone
	output PTT									// PTT active from key press, through delay to end of profile and hang
	);
	
profile_ROM	profile_ROM_inst (.address (profile_count),.clock (clock), .q(profile));


reg  [3:0]prof_state;
reg  [9:0]profile_count;
reg  [17:0] timer = 0;						// Holds start. Each clock = 5.2uS hence 192 clocks = 1mS
reg  char_PTT = 0;							// Max delay = 1023mS so need 192 * 1023 ~= 200,000 = 2^18 counts. 
reg  enable_hang = 0;


always @ (posedge clock)
begin 
case (prof_state)

0: 	begin
		 timer <= 0;
		 char_PTT <= 0;
       enable_hang <= 0;				
		 if (CW_char) begin  			// apply leading profile
			char_PTT <= 1'b1; 			// activate PTT from key press
			profile_count <= 0;
			if (delay == 0)		      // if no delay set then no delay required		
				prof_state <= 2;			// so go straight to profie generation
			else prof_state <= 1;			
		  end 
		end
		
1: begin										// delay for set mS
		if (timer == (delay * 192))begin
			timer <= 0;
			prof_state <= 2;
		end 
		else timer = timer + 18'd1;
	end 

2:	begin
		if (profile_count != 999) begin
			profile_count <= profile_count + 1'b1;
		end
		else begin
			if (!CW_char) 	begin 				// stay in this state until key is released
				if(delay == 0)						// delay not required
					prof_state <= 4;
				else prof_state <= 3;
			end 
		end
	end
	
3: begin
		if (timer == (delay * 192))begin  // extend element by delay time 
			timer <= 0;
			prof_state <= 4;
		end 
		else timer = timer + 18'd1;
	end
	
4: begin	
		if (profile_count != 0) 						// ROM address counter runs backwards
			profile_count <= profile_count - 1'b1;
		else begin
			if (hang != 0 )	enable_hang <= 1'b1;
			prof_state <= 0;
		end
	end
default: prof_state <= 0;
endcase
end


reg hang_PTT = 0;
reg hang_state = 0;
reg [17:0] hang_timer = 0;

// use separate code for hang so we can monitor key closures during this period 
always @ (negedge clock)
begin

case (hang_state)

0:	begin
		hang_PTT <= 0;
		hang_timer <= 0;
		if (enable_hang) begin				// see if hang delay is required
			hang_PTT <= 1'b1;					// active PTT due to hang delay
			hang_state <= 1;
		end
	end

1: begin 
		if (char_PTT) hang_timer <= 0;    // keep resetting timer whilst key is active
		else if (hang_timer == (hang * 192))		
			hang_state <= 0;
		else hang_timer <= hang_timer + 18'd1;
	end
	
default hang_state <= 0;
endcase	

end

assign PTT = char_PTT | hang_PTT;  		// PTT is set either by key closure or hang timer.

	
endmodule


