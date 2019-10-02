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



// ASMI_interface - copyright 2010, 2011 Phil Harman VK6APH

/*
	change log:


*/



module asmi_interface (clock, busy, erase, erase_ACK, IF_Rx_used, rdreq, IF_PHY_data,
					   erase_done, erase_done_ACK, send_more, send_more_ACK, num_blocks, NCONFIG); 

input 	wire 	clock;
input 	wire 	erase;
input 	wire 	[9:0] IF_Rx_used;
input  	wire  	erase_done_ACK;
input  	wire	send_more_ACK;
input   wire	[7:0] IF_PHY_data;
input   wire    [13:0] num_blocks;

output 	wire	busy;
output 	reg 	rdreq;
output 	reg 	erase_done;
output  reg		send_more;
output  reg		erase_ACK;
output  reg     NCONFIG;


reg		sector_erase;		// set to clear a sector of memory
reg 	[23:0]address; 		// address in EPCS16 to write to
reg 	write_enable; 
reg 	write;
reg     shift_bytes; 
reg 	[3:0]state;
reg 	[13:0]page; 		// counts number of 256 byte blocks we are writing 
reg 	[8:0]byte_count;	// holds number of bytes we have send to ASMI
reg     [21:0]reset_delay = 22'h0;  // delays reset after last frame sent


// reverse bit order into ASMI
wire [7:0]datain;
assign datain = {IF_PHY_data[0],IF_PHY_data[1],IF_PHY_data[2],IF_PHY_data[3],IF_PHY_data[4],IF_PHY_data[5],IF_PHY_data[6],IF_PHY_data[7]};

// state machine to manage ASMI inface

always @ (negedge clock) 
begin 
case (state)

0:	begin 
		sector_erase <= 0;				// reset sector erase
		write_enable <= 0;
		write <= 0;
		erase_ACK <= 0;
		erase_done <= 0;
		//address <= 24'h100000;			// set starting address to write to is top 1MB
		address <= 24'h0;
		byte_count <= 0;
		page <= 0;
		send_more <= 0;
		NCONFIG <= 0;
		if (erase) 
			 state <= 1'd1;
		else if(IF_Rx_used > 254) 		// if we have enough data in the Rx fifo then send to the EPCS16
			state <= 5;		 
	end

// do a sector erase	
1:	begin
		erase_ACK <= 1'b1; 	   	// let the Rx know we have seen the command
		write_enable <= 1'b1;
		sector_erase <= 1'b1;
		state <= 2;
	end 
// wait until erase has completed
2:	begin
		write_enable <= 0;
		sector_erase <= 0;
		if (busy) state <= 2;
		else if (address != 24'h1F0000) begin 
				address <= address + 24'h010000;
				state <= 1;
		end 
		else state <= 3;
	end
// let user know that erase has completed
3:	begin
		erase_done <= 1'b1;
		state <= 4;
	end 
// wait for the Tx to ack then return, loop here otherwise
4:	begin
		if (erase_done_ACK) state <= 0;
	end
	
// program EPCS16 with data from PC
5: 	begin 
	rdreq <= 1'b1;					
	byte_count <= 0; 				// reset byte counter
	state <= state + 1'b1;
	end 
// loop until we have sent 256 bytes
6:	begin
		if (byte_count == 9'd256) begin
			byte_count <= 0;				// reset byte counter
			shift_bytes <= 0;
			rdreq <= 0; 					// stop reading from fifo
			page <= page + 1'b1;			// increment the page counter
			state <= state + 1'b1;
		end 
		else begin
			write_enable <= 1'b1;			// enable write to the ASMI 
			shift_bytes <= 1'b1;			// enable loading data into the ASMI memory
			byte_count <= byte_count + 1'b1;
		end
	end 
// write the 256 bytes into the ASMI fifo and request more data
7:	begin
	write <= 1;
	state <= state + 1'b1;
	send_more <= 1'b1;		
	end 
// wait for request to be acknowledged and we have 255 more bytes ready then increment address by 256
// and get some more data 
8:	begin 
	write <= 0;
	write_enable <= 0;
		if (send_more_ACK) send_more <= 0;		// clear send flag once Tx has seen it
		if (page == num_blocks) begin			// all done so exit			
			 send_more <= 1'b1;					// let the PC know that the final block has been received.
			 state <= state + 1'b1;
		end 
		else if (!busy && IF_Rx_used > 254  && send_more == 0) begin
			address <= address + 24'd256;		// increment write address to point to next page boundry
			state <= 5;
		end
	end 

// delay so the final send_more is seen
9: 	begin
		if (send_more_ACK) begin 
			send_more <= 0;		
			state <= state + 1'b1;
		end 
	end 
	
// delay so the PC sees the final send more then when not busy reset the FPGA		
10:	begin
		if (&reset_delay) begin		
			if (!busy)							// wait until the last frame has been written then reset
				NCONFIG <= 1'b1;				// then reload FPGA from Flash	
		end 																
		else reset_delay <= reset_delay + 1'b1;
	end 
	
default: state <= 0;
		
endcase
end 


//----------------------------
// 			ASMI  
//----------------------------

//  If you enable wren in the MegaFunction then it must be set to 1 in order to write, protect or erase.

asmi_asmi_parallel_0 asmi_inst(
	.addr(address),
	.sector_erase(sector_erase),
	.busy(busy),
	.clkin(clock),
    .wren(write_enable),
    //.sector_protect(0),
	.datain(datain),
	.write(write),
	.shift_bytes(shift_bytes),
	.rden(1'b0),
	.read(1'b0),
	.reset(1'b0),
	.data_valid(),
	.dataout(),
	.illegal_erase(),
	.illegal_write()
); 


endmodule
