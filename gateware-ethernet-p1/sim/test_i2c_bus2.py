#!/usr/bin/env python


from myhdl import *
import os

import i2c

module = 'i2c_bus2'
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
module test_i2c_bus2;

// Parameters

// Inputs
reg clk = 0;
reg rst = 0;
reg scl_i = 1;
reg sda_i = 1;

reg [5:0]  wbs_adr_i;
reg [31:0] wbs_dat_i;
reg wbs_we_i;
reg wbs_stb_i; 
reg wbs_cyc_i;  


// Outputs
wire wbs_ack_o;
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
        sda_i,
        wbs_adr_i,
        wbs_dat_i,
        wbs_we_i,
        wbs_stb_i,
        wbs_cyc_i
    );
    $to_myhdl(
        wbs_ack_o,
        scl_o,
        scl_t,
        sda_o,
        sda_t
    );

    // dump file
    $dumpfile("test_i2c_bus2.lxt");
    $dumpvars(0, test_i2c_bus2);
end

i2c_bus2 UUT (
    .clk(clk),
    .rst(rst),
    .wbs_adr_i(wbs_adr_i),
    .wbs_dat_i(wbs_dat_i),
    .wbs_we_i(wbs_we_i),
    .wbs_stb_i(wbs_stb_i),
    .wbs_ack_o(wbs_ack_o),
    .wbs_cyc_i(wbs_cyc_i),
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

  wbs_adr_i = Signal(intbv(0)[6:])
  wbs_dat_i = Signal(intbv(0)[32:])
  wbs_we_i  = Signal(bool(0))
  wbs_stb_i = Signal(bool(0))
  wbs_ack_o = Signal(bool(0))
  wbs_cyc_i = Signal(bool(0))

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
  i2c_mem_inst1 = i2c.I2CMem(1024)

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

    wbs_adr_i = wbs_adr_i,
    wbs_dat_i = wbs_dat_i,
    wbs_we_i  = wbs_we_i,
    wbs_stb_i = wbs_stb_i,
    wbs_ack_o = wbs_ack_o,
    wbs_cyc_i = wbs_cyc_i,

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

    for i in range(0,20): yield clk.posedge

    wbs_adr_i.next = 0x3d
    wbs_dat_i.next = 0x06300605
    wbs_we_i.next = 0x1
    wbs_stb_i.next = 0x1
    wbs_cyc_i.next = 0x1
    print("Set values")

    for i in range(0,20):
      yield clk.posedge
      if wbs_ack_o: break

    wbs_adr_i.next = 0x0
    wbs_dat_i.next = 0x0
    wbs_we_i.next = 0x0
    wbs_stb_i.next = 0x0
    wbs_cyc_i.next = 0x0
    print("Reset values")

    for i in range(0,6000):
      yield clk.posedge

    wbs_adr_i.next = 0x3d
    wbs_dat_i.next = 0x06340605
    wbs_we_i.next = 0x1
    wbs_stb_i.next = 0x1
    wbs_cyc_i.next = 0x1
    print("Set values")

    for i in range(0,20):
      yield clk.posedge
      if wbs_ack_o: break

    wbs_adr_i.next = 0x0
    wbs_dat_i.next = 0x0
    wbs_we_i.next = 0x0
    wbs_stb_i.next = 0x0
    wbs_cyc_i.next = 0x0
    print("Reset values")

    for i in range(0,6000):
      yield clk.posedge

    wbs_adr_i.next = 0x00
    wbs_dat_i.next = 0x12345678
    wbs_we_i.next = 0x1
    wbs_stb_i.next = 0x1
    wbs_cyc_i.next = 0x1
    print("Set values")

    for i in range(0,20):
      yield clk.posedge
      if wbs_ack_o: break

    wbs_adr_i.next = 0x0
    wbs_dat_i.next = 0x0
    wbs_we_i.next = 0x0
    wbs_stb_i.next = 0x0
    wbs_cyc_i.next = 0x0
    print("Reset values")

    for i in range(0,10000):
      yield clk.posedge

    raise StopSimulation

  return dut, i2c_mem_logic1, bus, clkgen, check

def test_bench():
  sim = Simulation(bench())
  sim.run()

if __name__ == '__main__':
  print("Running test...")
  test_bench()
