#include "gateware.h"

FT_HANDLE ftHandle;

int init_gateware_ftdi() {

	printf("Init device for loading gateware...\r\n");

	FT_STATUS ftStatus = FT_OK;

	ftStatus = FT_OpenEx((void**)"radioberry-juice B", FT_OPEN_BY_DESCRIPTION, &ftHandle);
	
	if (ftStatus == FT_OK)
	{ // Port opened successfully
		unsigned char gateware_prog_io_mask = 0x07;

		//ftStatus |= FT_SetBitMode(ftHandle, 0, 0);
		ftStatus |= FT_SetBitMode(ftHandle, gateware_prog_io_mask, FT_BITMODE_ASYNC_BITBANG);
		ftStatus |= FT_SetLatencyTimer(ftHandle, 1);
		ftStatus |= FT_SetBaudRate(ftHandle, 1200000);  //Bit Bang mode is actually 16 times the Baud rate.
		ftStatus |= FT_SetUSBParameters(ftHandle, 65536, 65536);
		ftStatus |= FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0, 0);
		ftStatus |= FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
		ftStatus |= FT_SetTimeouts(ftHandle, 1000, 1000);

		if (ftStatus != FT_OK) {
			FT_Close(ftHandle);
			printf("Init device did not succeed for loading gateware.\r\n");
			return -1;
		}
	}
	else {
		printf("Opening FTDI device did not succeed.\r\n");
		return -1;
	}

	printf("Init device succeeded for loading gateware.\r\n");

	return 0;
};

int init_gateware_upload() {

	printf("Init FPGA gateware upload...\r\n");

	unsigned char outputBuffer[2];
	unsigned char inputBuffer[1];

	DWORD numBytesSent = 0;
	DWORD numBytesRead = 0;

	outputBuffer[0] = 0x00;		// set progam pins 'low'.
	outputBuffer[1] = NCONFIG;	// set nconfig program pin 'high' 


	if (FT_Write(ftHandle, outputBuffer, 2, &numBytesSent) != FT_OK) {
		printf("Init FPGA gateware upload write failed.\r\n");
		return -1;
	}

	if (FT_Read(ftHandle, inputBuffer, 1, &numBytesRead) != FT_OK) {
		printf("Init FPGA gateware upload read failed.\r\n");
		return -1;
	}

	//printf("Total bytes read: %ld value: %x\n", numBytesRead, inputBuffer[0]);

	while (!(inputBuffer[0] & NSTATUS)) {

		if (FT_Purge(ftHandle, FT_PURGE_RX) != FT_OK) {
			printf("Init FPGA gateware upload read failed (purge rx buffer).\r\n");
			return -1;
		}

		if (FT_Read(ftHandle, inputBuffer, 1, &numBytesRead) != FT_OK) {
			printf("Init FPGA gateware upload read failed.\r\n");
			return -1;
		}
		//printf("Total bytes read: %ld value: %x\n", numBytesRead, inputBuffer[0]);

		usleep(1000000);
	}

	printf("FPGA ready to receive gateware.\r\n");

	return 0;
}

int  fopen_s(FILE **f, const char *name, const char *mode) {
    int  ret = 0;
    *f = fopen(name, mode);
    /* Can't be sure about 1-to-1 mapping of errno and MS' errno_t */
    if (!*f)
        ret = errno;
    return ret;
}

int upload_gateware() {

	printf("FPGA gateware upload...\r\n");

	DWORD numBytesToSend = 0;
	DWORD numBytesToRead = 0;
	DWORD numBytesSent;
	DWORD numBytesRead;

	struct stat state;
	unsigned char bytebuf = 0x00;
	unsigned char* pdata;

	int err;

	int return_status = 0;

	FILE* fp;
	err = fopen_s(&fp, "radioberry.rbf", "rb");
	if (err != 0) {
		printf("The file 'radioberry.rbf' was not opened\n");
		return -1;
	}

	stat("radioberry.rbf", &state);
	
	printf("FPGA gateware size: %d\n", state.st_size);
	
	// we need 3 states per bit....
	int size = state.st_size * 8 * 3;

	printf("Actual size: %d\n", size);

	pdata = (unsigned char*)malloc(size);

	int byte = 0;

	SET_NCONFIG(bytebuf, 1); //NCONFIG must be kept high
	
	CLEAR_DCLK(bytebuf);	 //init clock bit to 0
	
	while ((byte = fgetc(fp)) != EOF)
	{
		
		for (int i = 0; i < 8; i++)
		{
			/* write from LSb to MSb */

			SET_DATA0(bytebuf, (byte & 0x01));
			pdata[numBytesToSend++] = bytebuf;

			//set clock bit to 1
			SET_DCLK(bytebuf, 1);
			pdata[numBytesToSend++] = bytebuf;

			//clear clock bit
			CLEAR_DCLK(bytebuf);
			pdata[numBytesToSend++] = bytebuf;

			//clear data bit
			CLEAR_DATA0(bytebuf);

			//shift current byte to the right
			byte >>= 1;
		}
		
		//not possible to send all data at once... so some bulk by bulk.
		if (numBytesToSend >= CHUNK_SIZE) {
			//printf("Send first total bytes to send: %ld\n", numBytesToSend);
		
			if (FT_Write(ftHandle, pdata, numBytesToSend, &numBytesSent) != FT_OK) {
				printf("Loading FPGA gateware failed.\r\n");
				return_status = -1;
			}

			//printf("Bytes to send: %ld and actual sent: %ld\n", numBytesToSend, numBytesSent);
			
			numBytesToSend = 0;
			
		}
	}
	
	if (FT_Write(ftHandle, pdata, numBytesToSend, &numBytesSent) != FT_OK) {
		printf("Loading FPGA gateware failed.\r\n");
		return_status = -1;
	}

	free(pdata);

	if (fclose(fp) != 0) printf("The file 'radioberry.rbf' was not closed\n");
	
	return return_status;
};

int activate_gateware() {

	printf("Activate FPGA gateware after upload...\r\n");

	unsigned char inputBuffer[1] ;

	DWORD numBytesRead = 0;

	if (FT_Purge(ftHandle, FT_PURGE_RX) != FT_OK) {
		printf("Activation gateware read failed (purge rx buffer).\r\n");
		return -1;
	}

	if (FT_Read(ftHandle, inputBuffer, 1, &numBytesRead) != FT_OK) {
		printf("Activation gateware read failed.\r\n");
		return -1;
	}

	while (!(inputBuffer[0] & NSTATUS) && !(inputBuffer[0] & NCONF_DONE) ) {

		if (FT_Purge(ftHandle, FT_PURGE_RX) != FT_OK) {
			printf("Activation gateware read failed (purge rx buffer).\r\n");
			return -1;
		}

		if (FT_Read(ftHandle, inputBuffer, 1, &numBytesRead) != FT_OK) {
			printf("Init FPGA gateware upload read failed.\r\n");
			return -1;
		}

		printf("NSTATUS and NCONF_DONE must be low...\n");

		usleep(1000);
	}

	DWORD numBytesToSend = 0;
	DWORD numBytesSent;

	unsigned char pdata[10];

	unsigned char bytebuf = 0x00;
	SET_NCONFIG(bytebuf, 1);	//NCONFIG must be kept high
	CLEAR_DATA0(bytebuf);		//clear data bit
	CLEAR_DCLK(bytebuf);		//init clock bit to 0
	pdata[numBytesToSend++] = bytebuf;

	for (int i = 0; i < 4; i++) {
		//set clock bit
		SET_DCLK(bytebuf, 1);
		pdata[numBytesToSend++] = bytebuf;

		//clear clock bit
		CLEAR_DCLK(bytebuf);
		pdata[numBytesToSend++] = bytebuf;
	}

	if (FT_Write(ftHandle, pdata, numBytesToSend, &numBytesSent) != FT_OK) {
		printf("Loading FPGA gateware failed.\r\n");
		return -1;
	}

	printf("FPGA gateware activated.\r\n");

	return 0;
};

int deinit_gateware_ftdi() {

	printf("Close device.\r\n");
	FT_STATUS ftStatus = FT_OK;
	
	ftStatus = FT_ResetDevice(ftHandle); 
	
	if (ftStatus == FT_OK) { 
	// FT_ResetDevice OK
	} 
	else { 
	// FT_ResetDevice failed 
	}
	

	if (FT_Close(ftHandle) != FT_OK) return -1;
	
	ftHandle = NULL;

	return 0;
};

unsigned long getFirmwareVersion() {
	FT_STATUS ftStatus;
	unsigned long dwDriverVer = 0;

	ftStatus = FT_GetDriverVersion(ftHandle,&dwDriverVer);

	return dwDriverVer;
}

int  load_gateware_image_into_fpga() {
	
		if (init_gateware_ftdi() < 0) return -1 ;
		init_gateware_upload();
		upload_gateware();
		activate_gateware();

	return 0;
	
}

// End of source.