#ifndef __RADIOBERRY_FILTERS_H__
#define __RADIOBERRY_FILTERS_H__

//***********************************************************************************
//		         Filters Board interface
//
//		   transport: I2C
//		   Protocols: Alex Board && VA2SAJ Generic Filters Interface && N2ADR board.
//
//***********************************************************************************
// Johan, PA3GSB modified using a i2c device; making it less platform dependent.


#include <linux/i2c-dev.h>

int fd_i2c_alex;
int fd_i2c_filter;
int fd_i2c_n2adr;

#define ADDR_N2ADR			0x20 		/* N2ADR is controlled using mcp23008*/
#define ADDR_ALEX 			0x21 		/* PCA9555 address 1 for Alex Interface Board*/
#define ADDR_FILTERS 		0x22 		/* Arduino filter board interface switcher address for VA2SAJ Generic Filter Switching Board*/

int i2c_n2adr_handler = 0;
int i2c_alex_handler = 0;
int i2c_filters_board_handler = 0;

int i2c_n2adr = 0;
int i2c_alex = 0;
int i2c_filters_board = 0;

int alex_manual = 0;
uint16_t i2c_alex_data = 0;
uint16_t i2c_data = 0;
unsigned int i2c_bus = 1;
int currentfreq = 4706000;
int previousfreq = 0;
int currentMox = 0;
int previousMox = -1;
int currentCW = 0;
int previousCW = -1;

//****************************************
//   Initializing Alex Interface
//****************************************
void initALEX() {
	int result = 0;
	unsigned char data[3];

	/* configure all pins as output */
	data[0] = 0x06;
	data[1] = 0x00;
	data[2] = 0x00;
	result = write(fd_i2c_alex, data, 3);

	if (result >= 0) {
		data[0] = 0x02;
		data[1] = 0x00;
		data[2] = 0x00;
		/* set all pins to low */
		result = write(fd_i2c_alex, data, 3);
	}

	if (result >= 0) {
		i2c_alex = 1;
	}
}
//*********************************************
//   Initializing Generic filters Interface
//*********************************************
void initGenericFilters() {
	int result = 0;
	unsigned char data[3];

	/* configure all pins as output */
	data[0] = 0x02;
	data[1] = 0x02;
	data[2] = 0x01;
	result =  write(fd_i2c_filter, data, 3);

	if (result >= 0) {
		data[0] = 0x02;
		data[1] = 0x02;
		data[2] = 0x03;
		/* set all pins to low */
		result =  write(fd_i2c_filter, data, 3);
	}

	if (result >= 0) {
		i2c_filters_board = 1;
	}
}

//****************************************
//   Initializing Alex Interface
//****************************************
void initN2ADR() {
	int result = 0;
	unsigned char data[2];

	/* configure all pins as output */
	data[0] = 0x00;
	data[1] = 0x00;
	result = write(fd_i2c_n2adr, data, 2);

	if (result >= 0) {
		data[0] = 0x09;
		data[1] = 0x00;
		/* set all pins to low */
		result = write(fd_i2c_n2adr, data, 2);
	}

	if (result >= 0) {
		i2c_n2adr = 1;
	}
}

//****************************************************************
//   Handle data to N2ADR Filter Board
// 
// https://github.com/softerhardware/Hermes-Lite2/wiki/Protocol
//
//  Look for Filter Selection description.
//****************************************************************
void handleN2ADRFilterBoard(char* buffer)
{
	if (i2c_filters_board & (buffer[523] & 0xFE) == 0x00) {
		i2c_alex_data = ((buffer[526] & 0x20) << 1) | ((buffer[525] & 0xFE) >> 1);
	}
	if (i2c_filters_board) {
		if (i2c_data != i2c_alex_data)
		{
			i2c_data = i2c_alex_data;
			
			unsigned char ldata[2];
			ldata[0] = 0x09;
			ldata[1] = i2c_alex_data & 0xFF;
			fprintf(stderr, "Set N2ADR data = %x \n", ldata[1]);

			write(fd_i2c_n2adr, ldata, 2);
		}
	}
}

//*******************************************
//   Handle data to Alex Interface Board
//*******************************************
void handleALEX(char* buffer)
{
	
	if (i2c_alex & (buffer[523] & 0xFE) == 0x12) {

		alex_manual = ((buffer[525] & 0x40) == 0x40) ? 1 : 0;
		if (alex_manual) {
			i2c_alex_data = ((buffer[526] & 0x8F) << 8) | (buffer[527] & 0xFF);
		}
		else {
			//firmware does determine the filter.
			uint16_t hpf = 0, lpf = 0;

			if (currentfreq < 1416000) hpf = 0x20; /* bypass */
			else if (currentfreq < 6500000) hpf = 0x10; /* 1.5 MHz HPF */
			else if (currentfreq < 9500000) hpf = 0x08; /* 6.5 MHz HPF */
			else if (currentfreq < 13000000) hpf = 0x04; /* 9.5 MHz HPF */
			else if (currentfreq < 20000000) hpf = 0x01; /* 13 MHz HPF */
			else hpf = 0x02; /* 20 MHz HPF */

			if (currentfreq > 32000000) lpf = 0x10; /* bypass */
			else if (currentfreq > 22000000) lpf = 0x20; /* 12/10 meters */
			else if (currentfreq > 15000000) lpf = 0x40; /* 17/15 meters */
			else if (currentfreq > 8000000) lpf = 0x01; /* 30/20 meters */
			else if (currentfreq > 4500000) lpf = 0x02; /* 60/40 meters */
			else if (currentfreq > 2400000) lpf = 0x04; /* 80 meters */
			else lpf = 0x08; /* 160 meters */

			i2c_alex_data = hpf << 8 | lpf;
		}
	}
	if (i2c_alex)
	{
		if (i2c_data != i2c_alex_data)
		{
			fprintf(stderr, "Set Alex data to output = %d \n", i2c_alex_data);
			i2c_data = i2c_alex_data;
			unsigned char ldata[3];
			ldata[0] = 0x02;
			ldata[1] = ((i2c_alex_data >> 8) & 0xFF);
			ldata[2] = (i2c_alex_data & 0xFF);
			fprintf(stderr, "Set Alex data 0 = %x \n", ldata[0]);
			fprintf(stderr, "Set Alex data 1 = %x \n", ldata[1]);
			fprintf(stderr, "Set Alex data 2 = %x \n", ldata[2]);
			write(fd_i2c_alex, ldata, 3);
		}
	}
}
//************************************************************************************************************************************
//                                 Handle data for Generic Filters Interface Board.
//                        Unlike Alex Interface, generic board uses frequency as data to send.
//    This allow easier integration for different lpf, bpf filter interface that didn't match Alex interface filters groups.
//    This also allow easier integration for different countries band plan because the band plan is defined in the arduino firmware.
//************************************************************************************************************************************
void handleFiltersBoard(char* buffer, int cw)
{
	if ((buffer[11] & 0xFE) == 0x04) {
		currentfreq = determine_freq(11, buffer);
	};
	if ((buffer[523] & 0xFE) == 0x04) {
		currentfreq = determine_freq(523, buffer);
	};
	
    //***********************************************
    //      Send Band Selected Alex board Style
    //***********************************************
    if (i2c_filters_board & (buffer[523] & 0xFE) == 0x12) {

        alex_manual = ((buffer[525] & 0x40) == 0x40) ? 1 : 0;
        if (alex_manual) {
            i2c_alex_data = ((buffer[526] & 0x8F) << 8) | (buffer[527] & 0xFF);
        } else {
            //firmware does determine the filter.
            uint16_t hpf = 0, lpf = 0;

            if (currentfreq < 1416000) hpf = 0x20; /* bypass */
            else if (currentfreq < 6500000) hpf = 0x10; /* 1.5 MHz HPF */
            else if (currentfreq < 9500000) hpf = 0x08; /* 6.5 MHz HPF */
            else if (currentfreq < 13000000) hpf = 0x04; /* 9.5 MHz HPF */
            else if (currentfreq < 20000000) hpf = 0x01; /* 13 MHz HPF */
            else hpf = 0x02; /* 20 MHz HPF */

            if (currentfreq > 32000000) lpf = 0x10; /* bypass */
            else if (currentfreq > 22000000) lpf = 0x20; /* 12/10 meters */
            else if (currentfreq > 15000000) lpf = 0x40; /* 17/15 meters */
            else if (currentfreq > 8000000) lpf = 0x01; /* 30/20 meters */
            else if (currentfreq > 4500000) lpf = 0x02; /* 60/40 meters */
            else if (currentfreq > 2400000) lpf = 0x04; /* 80 meters */
            else lpf = 0x08; /* 160 meters */

            i2c_alex_data = hpf << 8 | lpf;
        }
    }
    if (i2c_filters_board) {

        if (i2c_data != i2c_alex_data) {

            i2c_data = i2c_alex_data;
            unsigned char ldata[3];
            ldata[0] = 0x02;
            ldata[1] = ((i2c_alex_data >> 8) & 0xFF);
            ldata[2] = (i2c_alex_data & 0xFF);
            write(fd_i2c_filter, ldata, 3);
            fprintf(stderr, "Set Alex data 0 = %x \n", ldata[0]);
            fprintf(stderr, "Set Alex data 1 = %x \n", ldata[1]);
            fprintf(stderr, "Set Alex data 2 = %x \n", ldata[2]);
        }
    }
    //*************************************************
    //              Send Mox Status
    //*************************************************

    if (i2c_filters_board & ((buffer[11] & 0x01) == 0x01 || (buffer[11] & 0x01) == 0x00)) {
        currentMox = ((buffer[11] & 0x01) == 0x01) ? 1 : 0;
        currentCW = cw;
        if (currentMox != previousMox || currentCW != previousCW) {
            previousMox = currentMox;
            previousCW = currentCW;
           
            
            unsigned char ldata[3];
            ldata[0] = 0x03;
            ldata[1] = ((buffer[11] & 0x01) == 0x01) ? 1 : 0;
            ldata[2] = currentCW;
            write(fd_i2c_filter, ldata, 3);
            fprintf(stderr, "PTT data 0 = %x \n", ldata[0]);
            fprintf(stderr, "PTT data 1 = %x \n", ldata[1]);
            fprintf(stderr, "PTT data 2 = %x \n", ldata[2]);
        }
    }
    //*************************************************
    //     Send Frenquency to filter companion
    //*************************************************
    if (i2c_filters_board & (buffer[523] & 0xFE) == 0x12) {

        if (currentfreq != previousfreq) {

            previousfreq = currentfreq;
            unsigned tempFreq = currentfreq;
            unsigned char ldata[9];
            ldata[0] = 0x04;
            ldata[1] = (tempFreq / 10000000U) % 10;
            ldata[2] = (tempFreq / 1000000U) % 10;
            ldata[3] = (tempFreq / 100000U) % 10;
            ldata[4] = (tempFreq / 10000U) % 10;
            ldata[5] = (tempFreq / 1000U) % 10;
            ldata[6] = (tempFreq / 100U) % 10;
            ldata[7] = (tempFreq / 10U) % 10;
            ldata[8] = (tempFreq / 1U) % 10;

            write(fd_i2c_filter, ldata, 9);
            fprintf(stderr, "Set Filters frequency to = %d \n", currentfreq);

        }
    }  
        
}
//*******************************************
//   Convert frequency value to integer
//*******************************************
int determine_freq(int base_index, char* buffer) {
	return (((buffer[base_index + 1] & 0xFF) << 24) + ((buffer[base_index + 2] & 0xFF) << 16) + ((buffer[base_index + 3] & 0xFF) << 8) + (buffer[base_index + 4] & 0xFF));
}
//**********************************************************
//  Determine which board to forward data - Alex or Generic
//**********************************************************
void handleFilters(char* buffer, int cw) {
	if (i2c_alex) {
		handleALEX(buffer);
	}
	else if (i2c_filters_board) {
		handleFiltersBoard(buffer,cw);
	}
	else if (i2c_n2adr_handler) {
		handleN2ADRFilterBoard(buffer);
	}

}
//********************************************************************
//  Determine which filters interface is connected to the radioberry
//********************************************************************
void initFilters() {
	
	fd_i2c_alex = open("/dev/i2c-1", O_RDWR);
	fd_i2c_filter = open("/dev/i2c-1", O_RDWR);
	fd_i2c_n2adr = open("/dev/i2c-1", O_RDWR);
	
	if (fd_i2c_alex < 0 || fd_i2c_filter < 0 || fd_i2c_n2adr < 0) {
		fprintf(stderr, "Your SBC device is missing the following driver: '/dev/i2c-1' \n");
		fprintf(stderr, "To make use of an i2c speaking filterboard please load the driver first. \n");
		return;
	}
	
	i2c_alex_handler = ioctl(fd_i2c_alex, I2C_SLAVE, ADDR_ALEX);
	if (i2c_alex_handler >= 0) {
		initALEX();
	} else close(fd_i2c_alex);
	
	i2c_filters_board_handler = ioctl(fd_i2c_filter, I2C_SLAVE, ADDR_FILTERS);
	if (i2c_filters_board_handler >= 0) {	
		initGenericFilters();
	} else close(fd_i2c_filter);
	
	i2c_n2adr_handler = ioctl(fd_i2c_n2adr, I2C_SLAVE, ADDR_N2ADR);
	if (i2c_n2adr_handler >= 0) {	
		initN2ADR();
	} else close(fd_i2c_n2adr);

	if (i2c_alex) {
		fprintf(stderr, "alex interface found and initialized \n");
	}
	else if (i2c_filters_board) {
		fprintf(stderr, "Generic filters board interface found and initialized \n");
	}
	else if (i2c_n2adr_handler) {
		fprintf(stderr, "N2ADR filter board interface found and initialized \n");
	}
	else {
		fprintf(stderr, "No Alex, N2ADR or generic filters interface board connected to radioberry\n");
	}

}
#endif

