//*************************************************************************
//		         Filters Board interface
//
//		   transport: I2C
//		   Protocols: Alex Board && VA2SAJ Generic Filters Interface 
//
//*************************************************************************
#define ADDR_ALEX 			0x21 		/* PCA9555 address 1 for Alex Interface Board*/
#define ADDR_FILTERS 		0x20 	/* Arduino filter board interface switcher address for VA2SAJ Generic Filter Switching Board*/
int i2c_alex_handler = 0;
int i2c_filters_board_handler = 0;
int i2c_alex = 0;
int i2c_filters_board = 0;
int alex_manual = 0;
uint16_t i2c_alex_data = 0;
uint16_t i2c_data = 0;
unsigned int i2c_bus = 1;
int currentfreq = 4706000;
int previousfreq = 0;

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
	result = i2cWriteDevice(i2c_alex_handler, data, 3);

	if (result >= 0) {
		data[0] = 0x02;
		data[1] = 0x00;
		data[2] = 0x00;
		/* set all pins to low */
		result = i2cWriteDevice(i2c_alex_handler, data, 3);
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
	result = i2cWriteDevice(i2c_filters_board_handler, data, 3);

	if (result >= 0) {
		data[0] = 0x02;
		data[1] = 0x02;
		data[2] = 0x03;
		/* set all pins to low */
		result = i2cWriteDevice(i2c_filters_board_handler, data, 3);
	}

	if (result >= 0) {
		i2c_filters_board = 1;
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
			i2cWriteDevice(i2c_alex_handler, ldata, 3);
		}
	}
}
//************************************************************************************************************************************
//                                 Handle data for Generic Filters Interface Board.
//                        Unlike Alex Interface, generic board uses frequency as data to send.
//    This allow easier integration for different lpf, bpf filter interface that didn't match Alex interface filters groups.
//    This also allow easier integration for different countries band plan because the band plan is defined in the arduino firmware.
//************************************************************************************************************************************
void handleFiltersBoard(char* buffer)
{
	
	if (i2c_filters_board & (buffer[523] & 0xFE) == 0x12)
	{

		if (currentfreq != previousfreq)
		{
			fprintf(stderr, "Set Filters frequency to = %d \n", currentfreq);
			previousfreq = currentfreq;
			unsigned tempFreq = currentfreq;	
			unsigned char ldata[8];

			ldata[0] = (tempFreq / 10000000U) % 10;
			ldata[1] = (tempFreq / 1000000U) % 10;
			ldata[2] = (tempFreq / 100000U) % 10;
			ldata[3] = (tempFreq / 10000U) % 10;
			ldata[4] = (tempFreq / 1000U) % 10;
			ldata[5] = (tempFreq / 100U) % 10;
			ldata[6] = (tempFreq / 10U) % 10;
			ldata[7] = (tempFreq / 1U) % 10;

			i2cWriteDevice(i2c_filters_board_handler, ldata, 8);

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
void handleFilters(char* buffer) {

	if ((buffer[11] & 0xFE) == 0x04) {
		currentfreq = determine_freq(11, buffer);
	};
	if ((buffer[523] & 0xFE) == 0x04) {
		currentfreq = determine_freq(523, buffer);
	};
	if (i2c_alex) {
		handleALEX(buffer);
	}
	else if (i2c_filters_board) {
		handleFiltersBoard(buffer);
	}

}
//********************************************************************
//  Determine which filters interface is connected to the radioberry
//********************************************************************
void initFilters() {

	i2c_alex_handler = i2cOpen(i2c_bus, ADDR_ALEX, 0);
	if (i2c_alex_handler >= 0) {
		initALEX();
	}
	i2c_filters_board_handler = i2cOpen(i2c_bus, ADDR_FILTERS, 0);
	if (i2c_filters_board_handler >= 0) {	
		initGenericFilters();
	};

	if (i2c_alex) {
		fprintf(stderr, "alex interface found and initialized \n");
	}
	else if (i2c_filters_board) {
		fprintf(stderr, "Generic filters board interface found and initialized \n");
	}
	else {
		fprintf(stderr, "No alex or generic filters interface board connected to radioberry\n");
	}

}

