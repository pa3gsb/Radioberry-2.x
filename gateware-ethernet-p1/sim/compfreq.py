import numpy as np 

s = int(76.8e6)


def f2p1(f):
	phi = int(0x7fffffff *float(f)/(s/2))
	return hex(phi)

def f2p2(f):
	m2 = 1876499845
	m3 = 16777216
	phi = ((f*m2)+m3) >> 25
	return hex(phi)

