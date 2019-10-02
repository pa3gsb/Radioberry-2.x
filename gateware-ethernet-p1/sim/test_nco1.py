from myhdl import *
import os
import numpy as np
import spectrum
from scipy import signal

verilogtb = """
`timescale 1ps / 1ps

module test_nco1;

    reg  clk;
    reg  [31:0] phi;
    wire [18:0] cos;
    wire [18:0] sin;

    initial begin
        $from_myhdl(clk, phi);
        $to_myhdl(cos,sin);
        $dumpfile("test_nco1.fst");
        $dumpvars(0, test_nco1);
    end


nco1 #(.CALCTYPE(0)) UUT (
    .clk(clk),
    .rst(1'b0),
    .phi(phi), 
    .cos(cos),
    .sin(sin)
);

endmodule
"""



class Test(object):

    def __init__(self):

        f = open("test_nco1.v","w")
        f.write(verilogtb)
        f.close()

        self.sampling_freq = 76800000
        self.rx_freq = 7078132
        self.phi = int(0x7fffffff *float(self.rx_freq)/(self.sampling_freq/2))

        module = 'nco1'
        self.testbench = 'test_%s' % module

        srcs = []

        srcs.append("../rtl/nco/%s.v" % module)
        srcs.append("../rtl/nco/sincos.v")
        srcs.append("../rtl/nco/coarserom.v")
        srcs.append("../rtl/nco/finerom.v")
        srcs.append("%s.v" % self.testbench)

        src = ' '.join(srcs)

        self.build_cmd = "iverilog -DSIMULATION -g2012 -o %s.vvp %s" % (self.testbench,src)
        print(self.build_cmd)

        self.res = np.zeros(16384,'complex64')

        self.dt = 1.0/self.sampling_freq


    def bench(self):

        # Inputs
        clk = Signal(bool(0))
     
        maxv = 2**31
        phi = Signal(intbv(0,min=-maxv,max=maxv))
        maxv = 2**18
        sin = Signal(intbv(0,min=-maxv,max=maxv))
        cos = Signal(intbv(0,min=-maxv,max=maxv))

        # DUT
        if os.system(self.build_cmd):
            raise Exception("Error running build command")

        dut = Cosimulation(
            "vvp -m myhdl %s.vvp -fst" % self.testbench,
            clk=clk,
            phi=phi,
            sin=sin,
            cos=cos
        )

        @always(delay(int((self.dt*1e12)/2)))
        def clkgen():
            clk.next = not clk

        @instance
        def check():

            phi.next =  self.phi
           
            for i in range(32):
                yield negedge(clk)

            for i in range(16384):
                intcos = -cos[18:0] if cos[18] else cos[18:0]
                intsin = -sin[18:0] if sin[18] else sin[18:0]
                #print sin[18]
                self.res[i] = intcos + (1j * intsin)
                yield clk.negedge
     
            raise StopSimulation

        return dut, clkgen, check

    def test_bench(self):
        sim = Simulation(self.bench())
        sim.run()
        s = spectrum.Spectrum(self.res,self.dt,window=signal.flattop)
        peaks = s.findPeaks(order=4,clipdb=100)
        s.printPeaks(peaks)
        s.plot("NCO Spectrum")


if __name__ == '__main__':
    print("Running test...")
    t = Test()
    t.test_bench()

    
        


                

        

