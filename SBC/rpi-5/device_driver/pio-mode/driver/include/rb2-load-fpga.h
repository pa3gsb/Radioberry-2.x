#ifndef __RB2_LOAD_FPGA_H__
#define __RB2_LOAD_FPGA_H__

#include <linux/delay.h>


#define iPinCONF_DONE 		22	
#define iPinNSTATUS 		26	

#define oPinNCONFIG 		27	
#define oPinDATA 			13	
#define oPinDCLK 			24

void loading_radioberry_gateware(struct device *dev);

#endif

