

import numpy as np
import matplotlib as mpl
mpl.use('Qt4Agg',warn=False)
##mpl.rcParams['agg.path.chunksize'] = 1000000
import matplotlib.pyplot as plt
import pyfftw, sys

from scipy import signal

class Spectrum:

  def __init__(self,npa,dt,window=None):

    n = len(npa)

    self.complexinput = npa.dtype == 'complex64'
 

    if self.complexinput:
      fftia = pyfftw.n_byte_align_empty(n, 16, 'complex64')
      fftoa = pyfftw.n_byte_align_empty(n, 16, 'complex64')
    else:
      fftia = pyfftw.n_byte_align_empty(n, 16, 'float32')
      fftoa = pyfftw.n_byte_align_empty(int(n/2) + 1, 16, 'complex64')

    fft = pyfftw.FFTW(fftia,fftoa,flags=('FFTW_ESTIMATE',),planning_timelimit=60.0)

    maxv = npa.max()
    minv = npa.min()
    print("Max/Min is",maxv,minv)

    if window: 
      w = window(n)
      fftia[:] = w * (npa/maxv) 
    else:
      fftia[:] = npa/maxv

    fft()

    self.sa = np.abs(fftoa)

    ## Scale amplitude for window
    if window:
      scale = 1.0/np.sum(window(10000)/10000.0)
      print("Scaling postwindow by",scale)
      self.sa = scale * self.sa

    if self.complexinput:
      self.sa = np.concatenate( [self.sa[int(n/2):n],self.sa[0:int(n/2)]] ) 
    else:
      ## 2.0 To get magnitude in terms of original V since half of spectrum is returned
      self.sa = 2* self.sa
  
    ## Result is vrms
    print("Converting to dBFS")

    maxv = self.sa.max()
    print("Max is",maxv)
    print(self.sa)

    self.sa = 20.0*np.log10(self.sa/maxv) 

    if self.complexinput:
      self.mhz2bin = len(self.sa) * 1e6 * dt
    else:
      self.mhz2bin = len(self.sa) * 1e6 * 2 * dt

    self.bin2mhz = 1.0/self.mhz2bin

    print("Spectrum Array length is",len(self.sa))

  def binFreq(self,i):

    if self.complexinput:
      return (i - (len(self.sa)/2)) * self.bin2mhz
    else:
      return i * self.bin2mhz

  def findPeaks(self,order=2,clipdb=None):

    sa = self.sa

    if clipdb: 
        normcarrier = self.sa.max()
        sa = np.clip(sa,-clipdb,normcarrier+1)

    res = signal.argrelmax(sa,order=order)[0]

    peaks = []
    for i in res:
        peaks.append( (self.sa[i],self.binFreq(i)) )

    return peaks

  def printPeaks(self,peaks,maxtoprint=200):

    if len(peaks) > maxtoprint:
      print("Too many peaks to print")
      return
    else:
      print("Found {0} peaks".format(len(peaks)))

    print("|  MHz  |  dB  |")
    print("| -----:| ----:|")
    for (db,mhz) in peaks:
        print("| {0:10.6f} | {1:7.2f} |".format(mhz,db))

  def plot(self,title='Spectrum'):

    sa = self.sa
    n = len(sa)

    fig = plt.figure()
    fig.subplots_adjust(bottom=0.2)
    fig.suptitle(title, fontsize=20)
    sp = fig.add_subplot(111)

    if self.complexinput:
      xaxis = (np.r_[0:n] - (n/2)) * self.bin2mhz
    else:
      xaxis = np.r_[0:n] * self.bin2mhz

    sp.plot(xaxis,sa) ##,'-',color='b',label='Spectrum')
    sp.set_ylabel("dB")
    sp.set_xlabel("MHz")

    plt.show()              




class SpectrumIQ(Spectrum):

  def __init__(self,npa,dt,decimation,window=None):

    n = len(npa)
    print("Length n is",n)

    fftpg = 10*np.log10(n)
    print("FFT Processing Gain {0:.1f} dB".format(fftpg))
    filterpg = 10*np.log10(decimation)
    print("Filter Processing Gain {0:.1f} dB".format(filterpg))
    totalpg=fftpg+filterpg
    print("Total Processing Gain {0:.1f} dB".format(totalpg))

    fftia = pyfftw.n_byte_align_empty(n, 16, 'complex64')
    fftoa = pyfftw.n_byte_align_empty(n, 16, 'complex64')

    fft = pyfftw.FFTW(fftia,fftoa) ##,flags=('FFTW_ESTIMATE',),planning_timelimit=60.0)

    ra = np.real(npa)
    ia = np.imag(npa)
    bitsused = int(np.ceil(np.log2(2*np.max([np.max(ra),np.abs(np.min(ra)),np.max(ia),np.abs(np.min(ia))]))))
    print("Input bits in use",bitsused)
    #print("Real max={0:.1f} min={1:.1f}".format(np.max(ra),np.min(ra)))
    #print("Imag max={0:.1f} min={1:.1f}".format(np.max(ia),np.min(ia)))


    if window: 
      w = window(n)
      fftia[:] = w * npa
    else:
      fftia[:] = npa  


    fft()

    self.sa = np.abs(fftoa)

    ## Scale amplitude for window
    if window:
      scale = 1.0/np.sum(window(10000)/10000.0)
      print("Scaling postwindow by",scale)
      self.sa = scale * self.sa

    self.sa = np.concatenate( [self.sa[int(n/2):n],self.sa[0:int(n/2)]] )

    ## Result is vrms
    ##print("Converting to dBFS")

    maxv = self.sa.max()

    self.sa = 20.0*np.log10(self.sa/maxv) 

    w = int(n/4)
    print("Quarter noise floor means: {0:.1f} {1:.1f} {2:.1f} {3:.1f}".format(
      np.mean(self.sa[:w]),np.mean(self.sa[w:2*w]),np.mean(self.sa[2*w:3*w]),np.mean(self.sa[3*w:])))
 
    self.mhz2bin = len(self.sa) * 1e6 * dt * decimation
    self.bin2mhz = 1.0/self.mhz2bin

    print("Spectrum Array length is",len(self.sa))

