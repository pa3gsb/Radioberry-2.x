from myhdl import *
import os
import numpy as np
import spectrum
from scipy import signal

verilogtb = """
`timescale 1ps / 1ps

module test_mixtx1;

    reg  clk;
    reg  [31:0] phi;
    reg  signed [17:0] i_sig;
    reg  signed [17:0] q_sig;
    wire signed [12:0] i_result;
    wire signed [12:0] q_result;

    initial begin
        $from_myhdl(clk, phi, i_sig, q_sig);
        $to_myhdl(i_result,q_result);
        $dumpfile("test_mixtx1.fst");
        $dumpvars(0, test_mixtx1);
    end

mixtx1 UUT (
    .clk(clk),
    .rst(1'b0),
    .phi(phi),
    .i_sig(i_sig),
    .q_sig(q_sig), 
    .i_result(i_result),
    .q_result(q_result)
);

endmodule
"""



class Test(object):

    def __init__(self):

        f = open("test_mixtx1.v","w")
        f.write(verilogtb)
        f.close()

        self.sampling_freq = 76800000
        #self.rx_freq  = 7068001
        self.rx_freq  = 17280000
        #self.sig_freq = 7101023
        self.sig_freq = 100000
        self.phi = int(0x7fffffff *float(self.rx_freq)/(self.sampling_freq/2))

        module = 'mixtx1'
        self.testbench = 'test_%s' % module

        srcs = []

        srcs.append("../rtl/nco/%s.v" % module)
        srcs.append("../rtl/nco/nco1.v")
        srcs.append("../rtl/nco/sincos.v")
        srcs.append("../rtl/nco/coarserom.v")
        srcs.append("../rtl/nco/finerom.v")
        srcs.append("%s.v" % self.testbench)

        src = ' '.join(srcs)

        self.build_cmd = "iverilog -DSIMULATION -g2012 -o %s.vvp %s" % (self.testbench,src)
        print(self.build_cmd)

        ##self.res = np.zeros(16384,'complex64')
        self.res = np.zeros(16384)

        self.dt = 1.0/self.sampling_freq


    def bench(self):

        # Inputs
        clk = Signal(bool(0))
     
        maxv = 2**31
        phi = Signal(intbv(0,min=-maxv,max=maxv))
        maxv = 2**17
        i_sig  = Signal(intbv(0,min=-maxv,max=maxv))
        q_sig  = Signal(intbv(0,min=-maxv,max=maxv))
        maxv = 2**12
        i_result = Signal(intbv(0,min=-maxv,max=maxv))
        q_result = Signal(intbv(0,min=-maxv,max=maxv))

        # DUT
        if os.system(self.build_cmd):
            raise Exception("Error running build command")

        dut = Cosimulation(
            "vvp -m myhdl %s.vvp -fst" % self.testbench,
            clk=clk,
            phi=phi,
            i_sig=i_sig,
            q_sig=q_sig,
            i_result=i_result,
            q_result=q_result
        )

        @always(delay(int((self.dt*1e12)/2)))
        def clkgen():
            clk.next = not clk

        @instance
        def check():

            maxv = 0

            phi.next =  self.phi

            for i in range(32+16384):
                isig = np.sin(2*np.pi*self.sig_freq*i*self.dt)
                isig = int(round(isig * (2**17-1)))
                ##isig = 2**17-1
                i_sig.next = isig
                qsig = np.cos(2*np.pi*self.sig_freq*i*self.dt)
                qsig = int(round(qsig * (2**17-1)))
                ##qsig = 0
                q_sig.next = qsig

                if i>=32: 
                    ##self.res[i-32] = i_result + (1j * q_result) ##result
                    self.res[i-32] = q_result
                yield clk.negedge

            #print "Max value is",maxv,hex(maxv)
     
            raise StopSimulation

        return dut, clkgen, check

    def test_bench(self):
        sim = Simulation(self.bench())
        sim.run()
        s = spectrum.Spectrum(self.res,self.dt,window=signal.flattop)
        peaks = s.findPeaks(order=4,clipdb=95)
        s.printPeaks(peaks)
        s.plot("Mix Spectrum")


if __name__ == '__main__':
    print("Running test...")
    t = Test()
    t.test_bench()

    
        


                

        

