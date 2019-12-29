
import numpy as np
from scipy import signal


def sincos_coarse(coarsebits=10,resbits=18,maxoff=2):

  f = open("sincos_coarse.txt","w")

  for sv in range(2**(coarsebits-2)):
    x = (sv+0.5)/(2**coarsebits)

    sinx = np.sin(2*np.pi * x)
    sinx = int(np.round(sinx*(2**resbits-maxoff))) 

    cosx = np.cos(2*np.pi * x)
    cosx = int(np.round(cosx*(2**resbits-maxoff)))

    #print(sv,hex(sinx),hex(cosx))
    f.write("{0:018b}{1:018b} // {2} {3} {4}\n".format(sinx,cosx,sv,hex(sinx),hex(cosx)))

  f.close()

def sin_fine(coarsebits=10,finebits=10,resbits=18,maxoff=2):

  f = open("sin_fine.txt","w")

  for sv in range(2**(finebits-1)):

    x = (sv+0.5)/(2**(coarsebits+finebits))

    finesinx = np.sin(2*np.pi * x)
    finesinxf = int(np.round(finesinx*(2**resbits-maxoff)))
    finesinxt = int(np.round(finesinx*(2**(resbits-1)-maxoff))) 

    f.write("{0:09b} // {1} {2}\n".format(finesinxt,sv,hex(finesinxf)))
    #print(sv,hex(finesinxt))

  f.close()


sincos_coarse()

sin_fine()



