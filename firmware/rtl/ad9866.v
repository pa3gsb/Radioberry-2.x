//
//  Hermes Lite
// 
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

// (C) Steve Haynal KF7O 2014, 2015, 2016


module ad9866 (
    input reset,
    input clk,
    output reg sclk,
    output sdio,
    input sdo,
    output reg sen_n,
    output [7:0] dataout,
    input extrqst,
    input [5:0] gain
);


parameter bit [0:19][8:0] initarray = {
    // First bit is 1'b1 for write enable to that address
    {1'b1,8'h80}, // Address 0x00, enable 4 wire SPI
    {1'b0,8'h00}, // Address 0x01,
    {1'b0,8'h00}, // Address 0x02, 
    {1'b0,8'h00}, // Address 0x03, 
    {1'b0,8'h00}, // Address 0x04, 
    {1'b0,8'h00}, // Address 0x05, 
    {1'b0,8'h04}, // Address 0x06,
    {1'b1,8'h21}, // Address 0x07, Initiate DC offset calibration and RX filter on
    {1'b1,8'h4b}, // Address 0x08, RX filter f-3db at ~34 MHz after scaling
    {1'b0,8'h00}, // Address 0x09, 
    {1'b0,8'h00}, // Address 0x0a, 
    {1'b1,8'h20}, // Address 0x0b, RX gain only on PGA
    {1'b1,8'h41}, // Address 0x0c, TX twos complement and interpolation factor 
    {1'b1,8'h01}, // Address 0x0d, RT twos complement 
    {1'b0,8'h01}, // Address 0x0e, Enable/Disable IAMP 
    {1'b0,8'h00}, // Address 0x0f,     
    {1'b0,8'h84}, // Address 0x10, Select TX gain
    {1'b1,8'h00}, // Address 0x11, Select TX gain
    {1'b0,8'h00}, // Address 0x12, 
    {1'b0,8'h00}  // Address 0x13,     
};

reg [15:0] datain;
reg start;
reg [3:0] dut2_bitcount;
reg [0:0] dut2_state;
reg [15:0] dut2_data;
reg [5:0] dut1_pc;

logic [8:0] initarrayv;

// Init program counter
always @(posedge clk, posedge reset) begin: AD9866_DUT1_FSM
    if (reset == 1'b1) begin
        dut1_pc <= 6'h00;
    end
    else begin
        if ((dut1_pc != 6'h3f) & sen_n) begin
            dut1_pc <= (dut1_pc + 6'h01);
        end
        // Toggle LSB
        else if ((dut1_pc == 6'h3f) & sen_n) begin
            dut1_pc <= 6'h3e;
        end
    end
end

always @(sen_n, dut1_pc, gain, extrqst) begin: AD9866_DUT1_COMB
    initarrayv = {2'b01,gain};
    datain = {8'h0a,initarrayv[7:0]};   
    start = 1'b0;
    if (sen_n) begin
        if (dut1_pc[5:1] <= 6'h13) begin
            if (dut1_pc[0] == 1'b0) begin
                initarrayv = initarray[dut1_pc[5:1]];
                datain = {3'h0,dut1_pc[5:1],initarrayv[7:0]};
                start = initarrayv[8];
            end
        end else begin
            // Send gain code
            start = extrqst;
        end
    end
end

assign dataout = dut2_data[8-1:0];
assign sdio = dut2_data[15];

// SPI state machine
always @(posedge clk, posedge reset) begin: AD9866_DUT2_FSM
    if (reset == 1) begin
        sen_n <= 1;
        sclk <= 0;
        dut2_state <= 1'b0;
        dut2_data <= 0;
        dut2_bitcount <= 0;
    end
    else begin
        case (dut2_state)
            1'b0: begin
                sclk <= 0;
                dut2_bitcount <= 15;
                if (start) begin
                    dut2_data <= datain;
                    sen_n <= 0;
                    dut2_state <= 1'b1;
                end
                else begin
                    sen_n <= 1;
                end
            end
            1'b1: begin
                if ((!sclk)) begin
                    sclk <= 1;
                end
                else begin
                    dut2_data <= {dut2_data[15-1:0], sdo};
                    dut2_bitcount <= (dut2_bitcount - 1);
                    sclk <= 0;
                    if ((dut2_bitcount == 0)) begin
                        dut2_state <= 1'b0;
                    end
                end
            end
        endcase
    end
end

endmodule
