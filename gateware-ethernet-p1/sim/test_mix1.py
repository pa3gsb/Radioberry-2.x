from myhdl import *
import os
import numpy as np
import spectrum
from scipy import signal

verilogtb = """
`timescale 1ps / 1ps

module test_mix1;

    reg  clk;
    reg  [31:0] phi;
    reg  signed [11:0] adc;
    wire signed [17:0] i_data;
    wire signed [17:0] q_data;

    initial begin
        $from_myhdl(clk, phi, adc);
        $to_myhdl(i_data,q_data);
        $dumpfile("test_mix1.fst");
        $dumpvars(0, test_mix1);
    end

mix1 UUT (
    .clk(clk),
    .rst(1'b0),
    .phi(phi),
    .adc(adc), 
    .i_data(i_data),
    .q_data(q_data)
);

endmodule
"""



class Test(object):

    def __init__(self):

        f = open("test_mix1.v","w")
        f.write(verilogtb)
        f.close()

        self.sampling_freq = 76800000
        #self.rx_freq  = 7068001
        self.rx_freq  = 24800000
        #self.sig_freq = 7101023
        self.sig_freq = 24006123
        self.phi = int(0x7fffffff *float(self.rx_freq)/(self.sampling_freq/2))

        module = 'mix1'
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

        self.res = np.zeros(16384,'complex64')

        self.dt = 1.0/self.sampling_freq


    def bench(self):

        # Inputs
        clk = Signal(bool(0))
     
        maxv = 2**31
        phi = Signal(intbv(0,min=-maxv,max=maxv))
        maxv = 2**11
        adc  = Signal(intbv(0,min=-maxv,max=maxv))
        maxv = 2**17
        i_data = Signal(intbv(0,min=-maxv,max=maxv))
        q_data = Signal(intbv(0,min=-maxv,max=maxv))

        # DUT
        if os.system(self.build_cmd):
            raise Exception("Error running build command")

        dut = Cosimulation(
            "vvp -m myhdl %s.vvp -fst" % self.testbench,
            clk=clk,
            phi=phi,
            adc=adc,
            i_data=i_data,
            q_data=q_data
        )

        @always(delay(int((self.dt*1e12)/2)))
        def clkgen():
            clk.next = not clk

        @instance
        def check():

            maxv = 0

            phi.next =  self.phi

            for i in range(32+16384):
                sig = np.sin(2*np.pi*self.sig_freq*i*self.dt)
                sig = int(round(sig * (2**11-1)))
                adc.next = sig
                #adc.next = 0x07ff
                #if abs(i_data) > maxv: maxv = abs(i_data)
                #if abs(q_data) > maxv: maxv = abs(q_data)
                if i>=32: self.res[i-32] = i_data + (1j * q_data)
                yield clk.negedge

            #print "Max value is",maxv,hex(maxv)
     
            raise StopSimulation

        return dut, clkgen, check

    def test_bench(self):
        sim = Simulation(self.bench())
        sim.run()
        s = spectrum.Spectrum(self.res,self.dt,window=signal.flattop)
        peaks = s.findPeaks(order=4,clipdb=100)
        s.printPeaks(peaks)
        s.plot("Mix Spectrum")


if __name__ == '__main__':
    print("Running test...")
    t = Test()
    t.test_bench()

    
        


                

        

