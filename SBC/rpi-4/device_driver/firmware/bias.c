
#include "bias.h"
#include <sys/ioctl.h>

static int fd_i2c_bias;


static int mcp4662_present_probe(int fd) {
    uint8_t in[2];
    int n = read(fd, in, 2);
    return n == 2 ? 0 : -1;
}

void init_I2C_bias(void) {
	
	int i2c_bias_handler;
	
	fd_i2c_bias = open("/dev/i2c-1", O_RDWR);

	if (fd_i2c_bias < 0 ) {
		fprintf(stderr, "Your SBC device is missing the following driver: '/dev/i2c-1' \n");
		fprintf(stderr, "Change of Bias Setting is not possible\n");
		return ;// fd_i2c_bias;
	}
	i2c_bias_handler = ioctl(fd_i2c_bias, I2C_SLAVE, ADDR_BIAS);

	if (i2c_bias_handler < 0) {close(fd_i2c_bias);	fd_i2c_bias = -1; return;}
	
	if (mcp4662_present_probe(fd_i2c_bias) < 0) {
        close(fd_i2c_bias);
        fd_i2c_bias = -1;
		fprintf(stderr, "MCP 4662 not present; Change of Bias Setting is not possible\n");
    }
}

void write_I2C_bias(uint8_t control, uint8_t data) {
	
	uint8_t bias_data[2];
	bias_data[0] = control;
	bias_data[1] = data;
	
	
	if (fd_i2c_bias < 0 ) return;
	
	int result = write(fd_i2c_bias, bias_data, 2);
	
	if (result == 2) fprintf(stderr, "Write I2C Bias command %02X value= %02X \n", control, bias_data[1]); 
	else fprintf(stderr, "Write I2C Bias command failed \n"); 
}

void close_I2C_bias(void) {
	if (fd_i2c_bias >= 0) close(fd_i2c_bias);
}
//end of source
