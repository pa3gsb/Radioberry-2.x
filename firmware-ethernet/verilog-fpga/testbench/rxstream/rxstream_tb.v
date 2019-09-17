module rxstream_tb();


reg clk;
reg run;
reg have_ip;

reg udp_tx_enable;
wire udp_tx_request;
wire [7:0] udp_tx_data;
wire [10:0] udp_tx_length;

reg [47:0] rx_data;
wire rx_request;
reg [10:0] rx_length;


//UUT
rxstream rxstream_inst(
						.clk(clk), 
						//CONTROL STREAM
						.run(run),
						//ETH SETUP.
						.have_ip(have_ip),
						//UDP
						.udp_tx_enable(udp_tx_enable), 
						.udp_tx_request(udp_tx_request), 
						.udp_tx_data(udp_tx_data), 
						.udp_tx_length(udp_tx_length),
						//FIFO
						.rx_data(rx_data),
						.rx_request(rx_request),
						.rx_length(rx_length)
					);

initial
	begin: CLOCK_INIT
		$display ("clock init");
		clk = 0;
	end

always
	begin: CLOCK_GEN
		#5 clk = ~clk;
	end

always
	begin
		@ (posedge rx_request);
		#1 rx_data = {8'haf, 8'hae,  8'had, 8'hac, 8'hab, 8'haa};
		@ (posedge rx_request);
		#1 rx_data = {8'h16, 8'h15,  8'h14, 8'h13, 8'h12, 8'h11};
	end
	
always
	begin
		@ (posedge udp_tx_request);
		@ (posedge clk); 	//wait for clock
		udp_tx_enable = 1;	//start udp
		@ (posedge clk); 	//wait for clock
		udp_tx_enable = 0;
		
	end
	
initial
	begin
		#1 run = 0;
		#1 have_ip = 0;
		@ (posedge clk); 	//wait for clock
		#1 have_ip = 1;
		#1 run = 1;
		#1 rx_length = 17;	//not enough samples to start.
		@ (posedge clk); 	//wait for clock
		
		#1 rx_length = 172;	//enough samples to start.
		@ (posedge clk); 	//wait for clock

		@ (posedge udp_tx_request);	//run one packet
		@ (posedge udp_tx_request);	//run another packet
	
		#1 run = 0;			// stop running
		
	end

endmodule