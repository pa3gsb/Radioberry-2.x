


void spi_control_rx1_phase() {
	unsigned char iqdata[6];
	
	iqdata[0] = (0x10 | (sampleSpeed & 0x03));
	iqdata[1] = (((vna << 7) & 0x80) | ((rando << 6) & 0x40) | ((dither <<5) & 0x20) |  (att & 0x1F));
	iqdata[2] = ((rx1_phase >> 24) & 0xFF);
	iqdata[3] = ((rx1_phase >> 16) & 0xFF);
	iqdata[4] = ((rx1_phase >> 8) & 0xFF);
	iqdata[5] = (rx1_phase & 0xFF);
	
	sem_wait(&mutex);			
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
	sem_post(&mutex);
}

void spi_control_rx2_phase() {
	unsigned char iqdata[6];
	
	iqdata[0] = (0x20 | (sampleSpeed & 0x03));
	iqdata[1] = (((rando << 6) & 0x40) | ((dither <<5) & 0x20) |  (att & 0x1F));
	iqdata[2] = ((rx2_phase >> 24) & 0xFF);
	iqdata[3] = ((rx2_phase >> 16) & 0xFF);
	iqdata[4] = ((rx2_phase >> 8) & 0xFF);
	iqdata[5] = (rx2_phase & 0xFF);
			
	sem_wait(&mutex);			
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
	sem_post(&mutex);
}

void spi_control_tx() {
	unsigned char iqdata[6];
	
	iqdata[0] = 0x30;
	iqdata[1] = drive_level / 6.4;  // convert drive level from 0-255 to 0-39 )
	iqdata[2] = ((tx_phase >> 24) & 0xFF);
	iqdata[3] = ((tx_phase >> 16) & 0xFF);
	iqdata[4] = ((tx_phase >> 8) & 0xFF);
	iqdata[5] = (tx_phase & 0xFF);
		
	sem_wait(&mutex);		
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
	sem_post(&mutex);
}

void spi_send_control() {
	unsigned char iqdata[6];
	
	iqdata[0] = 0x40;
	iqdata[1] = pureSignal;
	iqdata[2] = ((pwmax >> 8) & 0x03);
	iqdata[3] = (pwmax & 0xFF);
	iqdata[4] = ((pwmin >> 8) & 0x03);
	iqdata[5] = (pwmin & 0xFF);
		
	sem_wait(&mutex);		
	spiXfer(rx1_spi_handler, iqdata, iqdata, 6);
	sem_post(&mutex);
}