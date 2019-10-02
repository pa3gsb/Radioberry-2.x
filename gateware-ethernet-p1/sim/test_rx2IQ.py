#!/usr/bin/env python3

#from myhdl import *
import os, sys
import numpy as np
import spectrum
import pickle
from spectrum import SpectrumIQ

from scipy import signal

verilogtb = """
    localparam VAR_ACC_WIDTH = (CICRATE == 10) ? 34 : (CICRATE == 13) ? 34 : (CICRATE == 5) ? 41 : 37; // Last is default rate of 8
    localparam CIC_ACC_WIDTH = (CICRATE == 10) ? 28 : (CICRATE == 13) ? 30 : (CICRATE == 5) ? 25 : 27; // Last is default rate of 8
    reg clk;
    reg flushed1;
    reg flushed2;
    reg signed [31:0] freq;
    reg signed [CORDIC_IN_WIDTH-1:0] in_data;
    wire signed [CORDIC_IN_WIDTH+CORDIC_EXTRA_BITS:0] out_data_I;
    wire signed [CORDIC_IN_WIDTH+CORDIC_EXTRA_BITS:0] in_data_I;
    wire signed [CORDIC_IN_WIDTH+CORDIC_EXTRA_BITS:0] out_data_Q;
    wire signed [CORDIC_IN_WIDTH+CORDIC_EXTRA_BITS:0] in_data_Q;    
    wire decimA_avail;
    wire decimB_avail;
    wire signed [CIC_OUT_WIDTH-1:0] decimA_realI;
    wire signed [CIC_OUT_WIDTH-1:0] decimA_realQ;    
    wire signed [CIC_OUT_WIDTH-1:0] out_decimA_realI;
    wire signed [CIC_OUT_WIDTH-1:0] out_decimA_realQ;
    wire signed [VAR_OUT_WIDTH-1:0] decimB_realI;
    wire signed [VAR_OUT_WIDTH-1:0] decimB_realQ;

    initial begin
        $from_myhdl(clk, freq, in_data, flushed1, flushed2);
        $to_myhdl(out_data_I,out_data_Q,decimA_avail,decimA_realI,decimA_realQ,decimB_avail,decimB_realI,decimB_realQ);
        $dumpfile("test_rx2.lxt");
        $dumpvars(0, test_rx2);
    end

cordic #(.IN_WIDTH(CORDIC_IN_WIDTH), .EXTRA_BITS(CORDIC_EXTRA_BITS)) cordic_i (
    .clock(clk), 
    .frequency(freq), 
    .in_data(in_data),
    .out_data_I(out_data_I),
    .out_data_Q(out_data_Q)           
);
assign in_data_I = flushed1 ? out_data_I : 0;
cic #(.STAGES(CIC_STAGES), .DECIMATION(CIC_DECIMATION), .IN_WIDTH(CORDIC_IN_WIDTH+CORDIC_EXTRA_BITS+1), .ACC_WIDTH(CIC_ACC_WIDTH), .OUT_WIDTH(CIC_OUT_WIDTH)) cic_i (
    .clock(clk),
    .in_strobe(1'b1),
    .out_strobe(decimA_avail),
    .in_data(in_data_I),
    .out_data(out_decimA_realI)
);
assign in_data_Q = flushed1 ? out_data_Q : 0;
cic #(.STAGES(CIC_STAGES), .DECIMATION(CIC_DECIMATION), .IN_WIDTH(CORDIC_IN_WIDTH+CORDIC_EXTRA_BITS+1), .ACC_WIDTH(CIC_ACC_WIDTH), .OUT_WIDTH(CIC_OUT_WIDTH)) cic_q (
    .clock(clk),
    .in_strobe(1'b1),
    .out_strobe(),
    .in_data(in_data_Q),
    .out_data(out_decimA_realQ)
);

assign decimA_realI = flushed2 ? out_decimA_realI : 0;
varcic #(.STAGES(VAR_STAGES), .IN_WIDTH(CIC_OUT_WIDTH), .ACC_WIDTH(VAR_ACC_WIDTH), .OUT_WIDTH(VAR_OUT_WIDTH), .CICRATE(CICRATE))
  varcic_i(
    .clock(clk),
    .in_strobe(decimA_avail),
    .decimation(VAR_DECIMATION),
    .out_strobe(decimB_avail),
    .in_data(decimA_realI),
    .out_data(decimB_realI)
    );

assign decimA_realQ = flushed2 ? out_decimA_realQ : 0;
varcic #(.STAGES(VAR_STAGES), .IN_WIDTH(CIC_OUT_WIDTH), .ACC_WIDTH(VAR_ACC_WIDTH), .OUT_WIDTH(VAR_OUT_WIDTH), .CICRATE(CICRATE))
  varcic_q(
    .clock(clk),
    .in_strobe(decimA_avail),
    .decimation(VAR_DECIMATION),
    .out_strobe(),
    .in_data(decimA_realQ),
    .out_data(decimB_realQ)
    );

endmodule
"""



class Test(object):

    def __init__(self,
        cordic_in_width=12,
        cordic_extra_bits=5,
        cic_stages=3,
        cic_out_width=14,
        cic_decimation=8,
        var_stages=5,
        var_out_width=16,
        var_decimation=12,
        ampl=1.0):

        self.cordic_in_width=cordic_in_width
        self.cordic_extra_bits=cordic_extra_bits
        self.cic_stages=cic_stages
        self.cic_out_width=cic_out_width
        self.cic_decimation=cic_decimation
        self.var_stages=var_stages
        self.var_out_width=var_out_width
        self.var_decimation=var_decimation

        f = open("test_rx2.v","w")
        f.write("`timescale 1ps / 1ps\nmodule test_rx2;\n")
        f.write("    localparam CORDIC_IN_WIDTH={0};\n".format(self.cordic_in_width))
        f.write("    localparam CORDIC_EXTRA_BITS={0};\n".format(self.cordic_extra_bits))
        f.write("    localparam CIC_STAGES={0};\n".format(self.cic_stages))
        f.write("    localparam CIC_OUT_WIDTH={0};\n".format(self.cic_out_width))
        f.write("    localparam CIC_DECIMATION={0};\n".format(self.cic_decimation))
        f.write("    localparam CICRATE={0};\n".format(self.cic_decimation))
        f.write("    localparam VAR_STAGES={0};\n".format(self.var_stages))
        f.write("    localparam VAR_OUT_WIDTH={0};\n".format(self.var_out_width))
        f.write("    localparam [5:0] VAR_DECIMATION={0};\n".format(self.var_decimation))


        f.write(verilogtb)
        f.close()

        self.sampling_freq = 73728000
        self.rx_freq = 5000000
        self.sigfreq = 5020000


        module = 'rx2'
        self.testbench = 'test_%s' % module

        srcs = []

        srcs.append("../rtl/cordic.v")
        srcs.append("../rtl/varcic.v")
        srcs.append("../rtl/cic.v")
        srcs.append("../rtl/cic_comb.v")
        srcs.append("../rtl/cic_integrator.v")

        srcs.append("%s.v" % self.testbench)

        src = ' '.join(srcs)

        self.build_cmd = "iverilog -g2012 -o %s.vvp %s" % (self.testbench,src)
        print(self.build_cmd)

        self.cordic_res = []
        self.cic_res = []
        self.varcic_res = []

        ## m2 and m3 from rtl
        ##m2 = 1876499845  76.8
        m2 = 1954687338  # 73.728
        m3 = 16777216

        self.rx_phase = (self.rx_freq * m2 + m3) >> 25

        self.dt = 1.0/self.sampling_freq

        self.offset = 0
        if ampl >= 0.0:
            self.scale = ampl*((2**(self.cordic_in_width-1))-1)    
        else:
            self.scale = abs(ampl)

        ## Handle 1 bit
        if self.scale <= 0.5: self.offset = self.scale


    def sig(self,i):

        return int(round( (self.scale*np.sin(2*np.pi*self.sigfreq*i*self.dt)+self.offset) ))

    def sig_withnoise(self,i):

        signal = self.scale*np.sin(2*np.pi*self.sigfreq*i*self.dt)
        noise = np.random.normal(loc=0.0, scale=10.0)
        dither = 400*np.sin(2*np.pi*7123456*i*self.dt)

        return int(round(signal+noise+dither))


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

        decimA_avail = Signal(bool(0))
        maxv = 2**(self.cic_out_width-1)
        decimA_realI = Signal(intbv(0,min=-maxv,max=maxv))
        decimA_realQ = Signal(intbv(0,min=-maxv,max=maxv))

        decimB_avail = Signal(bool(0))
        maxv = 2**(self.var_out_width-1)
        decimB_realI = Signal(intbv(0,min=-maxv,max=maxv))
        decimB_realQ = Signal(intbv(0,min=-maxv,max=maxv))

        flushed1 = Signal(bool(0))
        flushed2 = Signal(bool(0))

        # DUT
        if os.system(self.build_cmd):
            raise Exception("Error running build command")

        dut = Cosimulation(
            "vvp -m myhdl %s.vvp -lxt2" % self.testbench,
            clk=clk,
            flushed1=flushed1,
            flushed2=flushed2,
            freq=freq,
            in_data=in_data,
            out_data_I=out_data_I,
            out_data_Q=out_data_Q,
            decimA_avail=decimA_avail,
            decimA_realI=decimA_realI,
            decimA_realQ=decimA_realQ,
            decimB_avail=decimB_avail,
            decimB_realI=decimB_realI,
            decimB_realQ=decimB_realQ
        )

        @always(delay(int((self.dt*1e12)/2)))
        def clkgen():
            clk.next = not clk

        @instance
        def check():

            freq.next = self.rx_phase
            in_data.next = 0x00
            flushed1.next = 0x00
            flushed2.next = 0x00
           
            for i in range(128):
                if i == 20: flushed1.next = 0x1
                if i == 64: flushed2.next = 0x1
                in_data.next = self.sig(i)
                yield negedge(clk)

            i = 128
            j = 0
            while j < 2048:
                in_data.next = self.sig(i)
                i = i + 1
                if decimA_avail:
                    self.cic_res.append(int(decimA_realI) + -1j*int(decimA_realQ))
                if decimB_avail:
                    self.varcic_res.append(int(decimB_realI) + -1j*int(decimB_realQ))
                    j = j + 1
                self.cordic_res.append(int(out_data_I) + -1j*int(out_data_Q))
                yield clk.negedge
                if (i % 1024) == 0: print(j)
     
            raise StopSimulation

        return dut, clkgen, check

    def test_bench(self):
        sim = Simulation(self.bench())
        sim.run()
 
    def plot(self):
        s = SpectrumIQ(self.varcic_res,self.dt,self.cic_decimation*self.var_decimation,window=signal.blackmanharris)
        s.plot(title="VARCIC Spectrum",xoffset=int(len(self.varcic_res)/2))

        s = SpectrumIQ(self.cic_res[:8192],self.dt,self.cic_decimation,window=signal.blackmanharris)
        s.plot(title="CIC Spectrum",xoffset=int(8192/2))

        s = SpectrumIQ(self.cordic_res[:8192],self.dt,1,window=signal.blackmanharris)
        s.plot(title="Cordic Spectrum",xoffset=int(8192/2))




if __name__ == '__main__':
    if len(sys.argv) == 2:
        pf = open(sys.argv[1], 'rb')
        t = pickle.load(pf)
        t.plot()

    elif len(sys.argv) == 3 and sys.argv[2] == '-sim':
        t = Test(cic_out_width=14,ampl=-0.5)
        t.test_bench()
        pf = open(sys.argv[1], 'wb')
        pickle.dump(t,pf)
        pf.close()
        t.plot()

    else:
        print("usage: {0} <pickle_filename> [-sim]")


    
        


                

        

