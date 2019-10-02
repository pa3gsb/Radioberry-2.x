from myhdl import *
import os
import numpy as np
from scipy import signal

verilogtb = """

    reg clk;
    reg [19:0] angle;
    wire [18:0] cos;
    wire [18:0] sin;

    initial begin
        $from_myhdl(clk, angle);
        $to_myhdl(cos,sin);
        $dumpfile("test_sincos.lxt");
        $dumpvars(0, test_sincos);
    end


sincos UUT (
    .clk(clk), 
    .angle(angle), 
    .cos(cos),
    .sin(sin)
);

endmodule
"""


class Test(object):

    def __init__(self):

        f = open("test_sincos.v","w")
        f.write("`timescale 1ps / 1ps\nmodule test_sincos;\n")
        f.write(verilogtb)
        f.close()

        module = 'sincos'
        self.testbench = 'test_%s' % module

        srcs = []

        srcs.append("../rtl/nco/%s.v" % module)
        srcs.append("../rtl/nco/coarserom.v")
        srcs.append("../rtl/nco/finerom.v")
        srcs.append("%s.v" % self.testbench)

        src = ' '.join(srcs)

        self.build_cmd = "iverilog -g2012 -o %s.vvp %s" % (self.testbench,src)
        print(self.build_cmd)

        self.maxdiff = 0.0
        self.error = 0.0
        self.points = 2**20
        self.res = float(2**18)-1



    def bench(self):

        # Inputs
        clk = Signal(bool(0))
        angle = Signal(intbv(0,min=0,max=1048576))

        cos = Signal(intbv(0,min=0,max=524288))
        sin = Signal(intbv(0,min=0,max=524288))

        # DUT
        if os.system(self.build_cmd):
            raise Exception("Error running build command")

        dut = Cosimulation(
            "vvp -m myhdl %s.vvp -lxt2" % self.testbench,
            clk=clk,
            angle=angle,
            cos=cos,
            sin=sin
        )

        @always(delay(10))
        def clkgen():
            clk.next = not clk

        @instance
        def check():

            samples = 0
            error = 0.0

            angle.next = 0
            for i in range(8):
                yield  negedge(clk)

            for a in range(0,(2**20)/4):
                if (a % 10000) == 0:
                    print "Angle",a
                    if a > 15000: break
                angle.next = a
                for j in range(6):
                    yield negedge(clk)

                gold_cos = np.cos(2*np.pi * ((a+0.5)/self.points))
                gold_sin = np.sin(2*np.pi * ((a+0.5)/self.points))

                calc_cos = int(cos[18:0])/self.res
                calc_cos = -calc_cos if cos[18] else calc_cos
                calc_sin = int(sin[18:0])/self.res
                calc_sin = -calc_sin if sin[18] else calc_sin
                samples = samples + 1
                #print gold_cos,calc_cos
                #print gold_sin,calc_sin

                diff = abs((gold_cos**2) - (calc_cos**2))
                if diff > self.maxdiff: self.maxdiff = diff
                if diff > 0.01: print gold_cos,calc_cos,diff,angle
                error = error + diff
                ##print "Diff",diff,hex(int(cos[18:0]))
    
            self.error = error / samples

            for i in range(8):
                yield  negedge(clk)
     
            raise StopSimulation

        return dut, clkgen, check

    def test_bench(self):
        sim = Simulation(self.bench())
        sim.run()
        print "Average Error",self.error
        print "Max Diff",self.maxdiff
        

if __name__ == '__main__':
    print("Running test...")
    t = Test()
    t.test_bench()
