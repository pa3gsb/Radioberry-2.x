//------------------------------------------------------------------------------------------------------------------------------------------------------------ 
//
//				SPI Verilog (copied from opencores.org).
//

//------------------------------------------------------------------------------------------------------------------------------------------------------------
/* SPI MODE 3
		CHANGE DATA (sdout) @ NEGEDGE SCK
		read data (sdin) @posedge SCK
*/		

`include "timescale.v"


module spi_slave (rstb,ten,tdata,mlb,ss,sck,sdin, sdout,done,rdata);

  parameter WIDTH   = 48; 

  input rstb,ss,sck,sdin,ten,mlb;
  input [WIDTH-1:0] tdata;
  output sdout;           //slave out   master in 
  output reg done;
  output reg [WIDTH-1:0] rdata;

  reg [WIDTH-1:0] treg,rreg;
  reg [6:0] nb;
  wire sout;
  
  assign sout=mlb?treg[WIDTH-1]:treg[0];
  assign sdout=( (!ss)&&ten )?sout:1'bz; //if 1=> send data  else TRI-STATE sdout


//read from  sdout
always @(posedge sck or negedge rstb)
  begin
    if (rstb==0)
		begin rreg = 0;  rdata = 0; done = 0; nb = 0; end   //
	else if (!ss) begin 
			if(mlb==0)  //LSB first, in@msb -> right shift
				begin rreg ={sdin,rreg[WIDTH-1:1]}; end
			else     //MSB first, in@lsb -> left shift
				begin rreg ={rreg[WIDTH-2:0],sdin}; end  
		//increment bit count
			nb=nb+1;
			if(nb!=WIDTH) done=0;
			else  begin rdata=rreg; done=1; nb=0; end
		end	 //if(!ss)_END  if(nb==8)
  end

//send to  sdout
always @(negedge sck or negedge rstb)
  begin
	if (rstb==0)
		begin treg = 48'hFF; end
	else begin
		if(!ss) begin			
			if(nb==0) treg=tdata;
			else begin
			   if(mlb==0)  //LSB first, out=lsb -> right shift
					begin treg = {1'b1,treg[WIDTH-1:1]}; end
			   else     //MSB first, out=msb -> left shift
					begin treg = {treg[WIDTH-2:0],1'b1}; end			
			end
		end //!ss
	 end //rstb	
  end //always

endmodule