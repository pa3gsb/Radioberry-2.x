
#include "measure.h"

int i2c_measure_module_active;
int fd_i2c_measure;
int i2c_measure_handler;

int config_I2C_measure(){
	
	uint8_t measure_config[1];
	measure_config[0] = 0x07;

	int result = write(fd_i2c_measure, measure_config, 1);
	
	if (result == 1) fprintf(stderr, "Write Config to MAX11613 a 4 Channel ADC\n"); 
	else fprintf(stderr, "Radioberry amplifier config failed; only a problem if amplifier is installed. \n"); 
	
	return result;
}; 

void openI2C_measure() {
	
	i2c_measure_module_active = 0;
	
	fd_i2c_measure = open("/dev/i2c-1", O_RDWR);

	if (fd_i2c_measure < 0 ) {
		fprintf(stderr, "Your SBC device is missing the following driver: '/dev/i2c-1' \n");
		fprintf(stderr, "Measurement is not possible\n");
		return fd_i2c_measure;
	}
	i2c_measure_handler = ioctl(fd_i2c_measure, I2C_SLAVE, ADDR_MEAS);

	if (i2c_measure_handler >=0) if (config_I2C_measure()==1) i2c_measure_module_active = 1; else close(i2c_measure_handler);
};

void read_I2C_measure(int *current, int *temperature){
	
	uint8_t measure_data[8] ={0};
	
	int result = read(fd_i2c_measure, measure_data, 8);
	
	*temperature = (int)(((measure_data[2] & 0x0F) <<8) | measure_data[3]);
	*current = (int)(((measure_data[4] & 0x0F) <<8) | measure_data[5]);
};

void close_I2C_measure() {
	if (fd_i2c_measure != NULL) close(fd_i2c_measure);
};
//end of source