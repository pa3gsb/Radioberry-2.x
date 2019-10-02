#!/usr/bin/env python


from myhdl import *
import os

import i2c

module = 'slow_adc'
testbench = 'test_%s' % module

srcs = []

srcs.append("../rtl/%s.v" % module)
srcs.append("../rtl/i2c_master.v")
srcs.append("%s.v" % testbench)

src = ' '.join(srcs)

build_cmd = "iverilog -g2012 -o %s.vvp %s" % (testbench, src)

verilogtb = """
`timescale 1ns / 1ps

/*
 * Testbench for i2c_master
 */
module test_slow_adc;

// Parameters

// Inputs
reg clk = 0;
reg rst = 0;
reg [7:0] current_test = 0;
reg scl_i = 1;
reg sda_i = 1;

// Outputs
wire [11:0] ain0, ain1, ain2, ain3;
wire scl_o;
wire scl_t;
wire sda_o;
wire sda_t;


initial begin
    // myhdl integration
    $from_myhdl(
        clk,
        rst,
        scl_i,
        sda_i
    );
    $to_myhdl(
        ain0,
        ain1,
        ain2,
        ain3,
        scl_o,
        scl_t,
        sda_o,
        sda_t
    );

    // dump file
    $dumpfile("test_slow_adc.lxt");
    $dumpvars(0, test_slow_adc);
end

slow_adc UUT (
    .clk(clk),
    .rst(rst),
    .ain0(ain0),
    .ain1(ain1),
    .ain2(ain2),
    .ain3(ain3),
    .scl_i(scl_i),
    .scl_o(scl_o),
    .scl_t(scl_t),
    .sda_i(sda_i),
    .sda_o(sda_o),
    .sda_t(sda_t)
);

endmodule
"""
f = open(testbench+".v","w")
f.write(verilogtb)
f.close()

def bench():

    # Inputs
    clk = Signal(bool(0))
    rst = Signal(bool(0))

    ain0 = Signal(intbv(0)[12:])
    ain1 = Signal(intbv(0)[12:])
    ain2 = Signal(intbv(0)[12:])
    ain3 = Signal(intbv(0)[12:])

    scl_i = Signal(bool(1))
    sda_i = Signal(bool(1))
    scl_o = Signal(bool(1))
    scl_t = Signal(bool(1))
    sda_o = Signal(bool(1))
    sda_t = Signal(bool(1))

    s1_scl_i = Signal(bool(1))
    s1_sda_i = Signal(bool(1))
    s1_scl_o = Signal(bool(1))
    s1_scl_t = Signal(bool(1))
    s1_sda_o = Signal(bool(1))
    s1_sda_t = Signal(bool(1))


    # I2C memory model 1
    i2c_mem_inst1 = i2c.I2CModMem(1024)

    i2c_mem_logic1 = i2c_mem_inst1.create_logic(
        scl_i=s1_scl_i,
        scl_o=s1_scl_o,
        scl_t=s1_scl_t,
        sda_i=s1_sda_i,
        sda_o=s1_sda_o,
        sda_t=s1_sda_t,
        abw=1,
        address=0x34,
        latency=0,
        name='slave1'
    )

    # DUT
    if os.system(build_cmd):
        raise Exception("Error running build command")

    dut = Cosimulation(
        "vvp -m myhdl %s.vvp -lxt2" % testbench,
        clk=clk,
        rst=rst,

        ain0=ain0,
        ain1=ain1,
        ain2=ain2,
        ain3=ain3,

        scl_i=scl_i,
        scl_o=scl_o,
        scl_t=scl_t,
        sda_i=sda_i,
        sda_o=sda_o,
        sda_t=sda_t
    )

    @always_comb
    def bus():
        # emulate I2C wired AND
        ##print(scl_o)
        scl_i.next = scl_o & s1_scl_o;
        sda_i.next = sda_o & s1_sda_o;

        s1_scl_i.next = scl_o & s1_scl_o;
        s1_sda_i.next = sda_o & s1_sda_o;

    @always(delay(4))
    def clkgen():
        clk.next = not clk

    @instance
    def check():
        yield delay(100)
        yield clk.posedge
        rst.next = 1
        yield clk.posedge
        rst.next = 0
        yield clk.posedge
        yield delay(100)
 
        for i in range(0,20100):
            yield clk.posedge

        assert ain0==0x0708
        assert ain1==0x090a
        assert ain2==0x0b0c
        assert ain3==0x0d0e

        raise StopSimulation



    return dut, i2c_mem_logic1, bus, clkgen, check

def test_bench():
    sim = Simulation(bench())
    sim.run()

if __name__ == '__main__':
    print("Running test...")
    test_bench()
