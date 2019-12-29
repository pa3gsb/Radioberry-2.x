//------------------------------------------------------------------------------
//           Copyright (c) 2018 Johan Maas, PA3GSB
//
//			With special thanks to Rene Meerman PA3GUQ! 
//
//	The input data (IQ sample 48 bits) is requested from the FIFO ; after each edge controlled
//  by the firmware the data (8 bits) is provided to the output.
//------------------------------------------------------------------------------


module ddr_mux(
	 clk, 
	 reset,
	 rd_req,
	 in_data,
	 out_data);

	input clk;
	input reset;
	output rd_req;
	input [47:0] in_data;
	output [7:0] out_data;
	
	reg[15:0] mux16;	
	reg[1:0] mux_sel;
	reg[7:0] data_p;
	reg[7:0] data_n;					
								
	always @(negedge clk)
	begin
	  if (reset) begin
		mux_sel <= 0;
		rd_req <= 0;
	  end else begin
		rd_req <= 0;
		mux_sel <= mux_sel +1;
		if (mux_sel == 2) begin
		  mux_sel <= 0;
		  rd_req <= 1;
		end;
	  end
	end

	always @(mux_sel)
	begin
	  case (mux_sel)
		0: begin mux16 <= in_data[47:32]; end 
		1: begin mux16 <= in_data[31:16]; end
		2: begin mux16 <= in_data[15: 0]; end
		default:
		  mux16 <= 0;
	  endcase
	end

	always @(posedge clk)
	begin
	  if (reset) begin
		data_p <= 0;
	  end else begin
		data_p <= mux16[15:8];
	  end
	end

	always @(negedge clk)
	begin
	  if (reset) begin
		data_n <= 0;
	  end else begin
		data_n <= mux16[7:0];
	  end
	end

	assign out_data = (clk == 1) ? data_p : data_n; 
	
endmodule