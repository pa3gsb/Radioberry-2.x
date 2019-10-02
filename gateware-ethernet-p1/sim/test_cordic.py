from myhdl import *
import os
import numpy as np
import spectrum
from scipy import signal

verilogtb = """

    reg clk;
    reg signed [31:0] freq;
    reg signed [CORDIC_IN_WIDTH-1:0] in_data;
    wire signed [CORDIC_IN_WIDTH+CORDIC_EXTRA_BITS:0] out_data_I;
    wire signed [CORDIC_IN_WIDTH+CORDIC_EXTRA_BITS:0] out_data_Q;

    initial begin
        $from_myhdl(clk, freq, in_data);
        $to_myhdl(out_data_I,out_data_Q);
        $dumpfile("test_cordic.lxt");
        $dumpvars(0, test_cordic);
    end


cordic #(.IN_WIDTH(CORDIC_IN_WIDTH), .EXTRA_BITS(CORDIC_EXTRA_BITS)) UUT (
    .clock(clk), 
    .frequency(freq), 
    .in_data(in_data),
    .out_data_I(out_data_I),
    .out_data_Q(out_data_Q)           
);

endmodule
"""


class Test(object):

    def __init__(self,
        cordic_in_width=12,
        cordic_extra_bits=5,
        ampl=1.0):

        self.cordic_in_width=cordic_in_width
        self.cordic_extra_bits=cordic_extra_bits


        f = open("test_cordic.v","w")
        f.write("`timescale 1ps / 1ps\nmodule test_cordic;\n")
        f.write("    localparam CORDIC_IN_WIDTH={0};\n".format(self.cordic_in_width))
        f.write("    localparam CORDIC_EXTRA_BITS={0};\n".format(self.cordic_extra_bits))
        f.write(verilogtb)
        f.close()

        self.sampling_freq = 76800000
        self.rx_freq = 20480000 ##28000000
        self.sigfreq = 20590000 #27500000


        module = 'cordic'
        self.testbench = 'test_%s' % module

        srcs = []

        srcs.append("../rtl/radio_openhpsdr1/%s.v" % module)
        srcs.append("%s.v" % self.testbench)

        src = ' '.join(srcs)

        self.build_cmd = "iverilog -g2012 -o %s.vvp %s" % (self.testbench,src)
        print(self.build_cmd)

        self.res = np.zeros(16384,'complex64')


        ## m2 and m3 from rtl
        m2 = 1876499845
        m3 = 16777216

        self.rx_phase = (self.rx_freq * m2 + m3) >> 25
        ##self.rx_phase = 0x60103001

        self.dt = 1.0/self.sampling_freq

        self.offset = 0
        if ampl >= 0.0:
            self.scale = ampl*((2**(self.cordic_in_width-1))-1)    
        else:
            self.scale = abs(ampl)

        ## Handle 1 bit
        if self.scale <= 0.5: self.offset = self.scale




    def Sig(self,i):

        return int(round( (self.scale*np.cos(2*np.pi*self.sigfreq*i*self.dt)+self.offset) ))

    def bench(self):

        # Inputs
        clk = Signal(bool(0))
     
        maxv = 2**31
        freq = Signal(intbv(0,min=-maxv,max=maxv))
        maxv = 2**(self.cordic_in_width+self.cordic_extra_bits+1-1)
        out_data_I = Signal(intbv(0,min=-maxv,max=maxv))
        out_data_Q = Signal(intbv(0,min=-maxv,max=maxv))
        maxv = 2**(self.cordic_in_width-1)
        in_data = Signal(intbv(0,min=-maxv,max=maxv))

        # DUT
        if os.system(self.build_cmd):
            raise Exception("Error running build command")

        dut = Cosimulation(
            "vvp -m myhdl %s.vvp -lxt2" % self.testbench,
            clk=clk,
            freq=freq,
            in_data=in_data,
            out_data_I=out_data_I,
            out_data_Q=out_data_Q
        )

        @always(delay(int((self.dt*1e12)/2)))
        def clkgen():
            clk.next = not clk

        @instance
        def check():

            freq.next = self.rx_phase
            in_data.next = 0x00
           
            for i in range(32):
                in_data.next = self.Sig(i)
                yield negedge(clk)

            for i in range(16384):
                in_data.next = self.Sig(i+32)
                self.res[i] = out_data_I + (1j * out_data_Q)
                yield clk.negedge
     
            raise StopSimulation

        return dut, clkgen, check

    def test_bench(self):
        sim = Simulation(self.bench())
        sim.run()
        s = spectrum.Spectrum(self.res,self.dt,window=signal.flattop)
        peaks = s.findPeaks(order=1,clipdb=100)
        s.printPeaks(peaks)
        s.plot("Cordic Spectrum")


if __name__ == '__main__':
    print("Running test...")
    t = Test(ampl=1.0)
    t.test_bench()
    ##t = Test(16,7)
    ##t.test_bench()

    
        


                

        

