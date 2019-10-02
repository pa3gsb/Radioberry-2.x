
import numpy as np
import spectrum, random
from scipy import signal



class CosSin2:

  def __init__(self,coarsebits=10,finebits=10,maxoff=2,resbits=18,fineresstriplsb=1):

    self.coarsebits = coarsebits
    self.finebits = finebits
    self.maxoff = maxoff
    self.resbits = resbits
    self.fineresstriplsb = fineresstriplsb

    self.res = np.zeros(2**18,'complex64')

    self.sampling_freq = 76800000
    self.dt = 1.0/self.sampling_freq

    self.trueres = False

    self.coarse_mask = 2**(self.coarsebits-2)-1


  def cos_coarse(self,v):
    if not (0 <= v <= 2**self.coarsebits-1):
      print("v not valid for specified coarse bits",v)
      return 0

    ctrl = v >> (self.coarsebits-2)

    sv = v & self.coarse_mask

    sv = (sv ^ self.coarse_mask) if (ctrl & 0x01) else sv

    x = np.cos(2*np.pi * ((sv+0.5)/2**self.coarsebits))
    res = int(np.round(x*(2**self.resbits-self.maxoff))) 

    sign = ((ctrl & 0x01) ^ ((ctrl >> 1) & 0x01)) > 0

    return sign,res


  def sin_coarse(self,v):
    if not (0 <= v <= 2**self.coarsebits-1):
      print("v not valid for specified coarse bits",v)
      return 0

    ctrl = v >> (self.coarsebits-2)

    sv = v & self.coarse_mask

    sv = (sv ^ self.coarse_mask) if (ctrl & 0x01) else sv

    x = np.sin(2*np.pi * ((sv+0.5)/2**self.coarsebits))
    res = int(np.round(x*(2**self.resbits-self.maxoff))) 

    sign = (ctrl & 0x02) > 0

    return sign,res

  def sin_fine_ram(self,v):
    if not (0 <= v <= 2**self.finebits-1):
      print("v not valid for specified fine bits",v)
      return 0

    svmask = 2**(self.finebits-1)-1

    sv = v & svmask

    negate = v<=sv

    sv = (sv ^ svmask) if negate else sv

    ##print "S2",hex(sv)

    x = np.sin(2*np.pi * ((sv+0.5)/2**(self.coarsebits+self.finebits)))
    
    if self.fineresstriplsb:
      res = int(np.round(x*(2**(self.resbits-1)-self.maxoff)))
      res = res << 1
    else:
      res = int(np.round(x*(2**self.resbits-self.maxoff)))

    return negate,res,self.resbits

  ## Set finebits to 9 for 9x9 multiplier
  def sin_fine_mult(self,v):
    if not (0 <= v <= 2**self.finebits-1):
      print("v not valid for specified fine bits",v)
      return 0

    svmask = 2**(self.finebits-1)-1

    sv = v & svmask

    negate = v<=sv

    sv = (sv ^ svmask) if negate else sv

    # 402 is 2*pi*64 to fit in 9 bits, 2(sv+1), 403 provides slight improvement
    res = 403 * ((sv <<1) | 0x01)
    rshift = self.resbits + (self.finebits-1)

    #n = (402 * sv) + 201
    #n = n >> 6

    ## 8*8 with 8 bit result
    #res =202 * ((sv <<1) | 0x01)
    #res = res >> 8 
    #rshift = self.resbits -2 
    #rshift = self.resbits + (self.finebits -2)

    ## 7x7 with 8 bit result
    ##res =101 * ((sv <<1) | 0x01)
    ##res = res >> 6
    ##rshift = self.resbits - 2
    #rshift = self.resbits + (self.finebits -3)


    return negate,res,rshift

  def comp1(self,v):
    v = v >> (32 - (self.coarsebits+self.finebits))
    v = v & (2**(self.coarsebits+self.finebits)-1)

    cc,cos_coarse = self.cos_coarse(v >> self.finebits)
    sc,sin_coarse = self.sin_coarse(v >> self.finebits)
    sf, sin_fine, rshift = self.sin_fine_mult(v & (2**self.finebits - 1))

    ##print "S1",hex(v &(2**self.finebits - 1)),hex(sin_fine)

    scsf = (sin_coarse*sin_fine) >> rshift
    ccsf = (cos_coarse*sin_fine) >> rshift
    scv  = sin_coarse
    ccv  = cos_coarse 
    
    ##print "S2",hex(sin_fine),hex(scsf),hex(ccsf),hex(scv),hex(ccv)

    ## Absolute value of correction factor is always smaller
    ##cos = (ccv - scsf)
    if (cc,sc,sf) == (False,False,False):
      cos = ccv - scsf
    elif (cc,sc,sf) == (False,False,True):
      cos = ccv + scsf
    elif (cc,sc,sf) == (False,True,False):
      cos = ccv + scsf
    elif (cc,sc,sf) == (False,True,True):
      cos = ccv - scsf
    elif (cc,sc,sf) == (True,False,False):
      cos = ccv + scsf
    elif (cc,sc,sf) == (True,False,True):
      cos = ccv - scsf
    elif (cc,sc,sf) == (True,True,False):
      cos = ccv - scsf
    elif (cc,sc,sf) == (True,True,True):
      cos = ccv + scsf

    ##sin = (scv + ccsf) 
    if (sc,cc,sf) == (False,False,False):
      sin = scv + ccsf
    elif (sc,cc,sf) == (False,False,True):
      sin = scv - ccsf
    elif (sc,cc,sf) == (False,True,False):
      sin = scv - ccsf
    elif (sc,cc,sf) == (False,True,True):
      sin = scv + ccsf
    elif (sc,cc,sf) == (True,False,False):
      sin = scv - ccsf
    elif (sc,cc,sf) == (True,False,True):
      sin = scv + ccsf
    elif (sc,cc,sf) == (True,True,False):
      sin = scv + ccsf
    elif (sc,cc,sf) == (True,True,True):
      sin = scv - ccsf

    if cos >= 2**18:
      print "Overflow cos",v,hex(cos),hex(ccv),hex(scsf)

    ##if sin >= 2**18:
    ##  print "Overflow sin",v,sin

      
    ## Convert back to 17 bit resolution 2's complement
    if self.trueres:
      cos = int(cos / 2) << 1
      sin = int(sin / 2) << 1
    ##print "S3",hex(cos)

    cos = cos /float(2**self.resbits-self.maxoff)
    cos = -cos if cc else cos
    sin = sin /float(2**self.resbits-self.maxoff)
    sin = -sin if sc else sin


    return cos + (1j*sin)


  def freqtest(self,phase):
    a = 0
    for i in range(2**18):
      self.res[i] = self.comp1(a)
      a += phase
      a = a & 0x0ffffffff
      ## dither
      ##
      ##a = a ^ random.randint(0,255)
    s = spectrum.Spectrum(self.res,self.dt,window=signal.flattop)
    peaks = s.findPeaks(order=4,clipdb=120)
    s.printPeaks(peaks)
    s.plot("NCO Spectrum")


  def mixtest(self,phase):
    a = 0
    for i in range(2**18):
      sig = np.cos(2*np.pi*24006123*i*self.dt)
      sig = int(round(sig * 2**11)) / float(2**11)
      res = sig * self.comp1(a)
      self.res[i] = res

      #rr = int((res.real * 2**17))/ float(2**17)
      #ri = int((res.imag * 2**17))/ float(2**17)
      #self.res[i] = (rr + (ri*1j))
      a += phase
      a = a & 0x0ffffffff
      ## dither
      ##
      ##a = a ^ random.randint(0,255)
    s = spectrum.Spectrum(self.res,self.dt,window=signal.flattop)
    peaks = s.findPeaks(order=4,clipdb=100)
    s.printPeaks(peaks)
    s.plot("NCO Spectrum")


  def accuracy(self):
    maxdiff = 0.0
    error = 0.0
    points = 2**(self.coarsebits+self.finebits)
    samples = points/4
    shift = 32 - (self.coarsebits+self.finebits)
    for i in range(samples):
      comp = self.comp1(i << shift)
      calc = np.cos(2*np.pi * ((i+0.5)/points))
      diff = abs((comp.real**2) - (calc**2))
      if diff > maxdiff: maxdiff = diff
      error = error + diff
    avgerror = error / samples
    print avgerror,maxdiff
 
  def smalltest(self):
    maxdiff = 0.0
    error = 0.0
    points = 2**(self.coarsebits+self.finebits)
    samples = 0
    shift = 32 - (self.coarsebits+self.finebits)
    for i in range(600000,601000,200):
      samples = samples + 1
      comp = self.comp1(i << shift)
      calc = np.cos(2*np.pi * ((i+0.5)/points))
      diff = abs((comp.real**2) - (calc**2))
      ##print "Diff",diff
      if diff > maxdiff: maxdiff = diff
      error = error + diff
    avgerror = error / samples
    print avgerror,maxdiff


cs = CosSin2()


rx_freq = 23000123
phi = int(0x7fffffff *float(rx_freq)/(cs.sampling_freq/2))
#cs.test()
#cs.compare()
##cs.freqtest(0x2ee9c123)
##cs.freqtest(0x3ef23037)
##cs.accuracy()
cs.mixtest(phi)
#cs.freqtest(0x5fffffff)
#for f in range(0x1e,0x4f):
#  f2 = (f << 24) | 0x531471
#  print hex(f2)
#  cs.freqtest(f2)
