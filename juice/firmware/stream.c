
#include "stream.h"

FT_HANDLE ftHandleA;

int init_stream() {

	printf("Init device for iq stream handling...\r\n");

	FT_STATUS ftStatus = FT_OK;

	ftStatus = FT_OpenEx((void**)"radioberry-juice A", FT_OPEN_BY_DESCRIPTION, &ftHandleA);

	if (ftStatus == FT_OK)
	{ // Port opened successfully
		ftStatus |= FT_SetBitMode(ftHandleA, 0, 0);
		usleep(1000000);
		ftStatus |= FT_SetBitMode(ftHandleA, 0xFF, FT_BITMODE_SYNC_FIFO);
		ftStatus |= FT_SetLatencyTimer(ftHandleA, 1);
		ftStatus |= FT_SetUSBParameters(ftHandleA, 65536, 65536);
		ftStatus |= FT_SetFlowControl(ftHandleA, FT_FLOW_RTS_CTS, 0, 0);
		ftStatus |= FT_Purge(ftHandleA, FT_PURGE_RX | FT_PURGE_TX);
		ftStatus |= FT_SetTimeouts(ftHandleA, 1000, 1000);

		if (ftStatus != FT_OK) {
			FT_Close(ftHandleA);
			printf("Init device did not succeed for iq streaming.\r\n");
			return -1;
		}
	}
	else {
		printf("Opening FTDI device did not succeed.\r\n");
		return -1;
	}

	printf("Init device succeeded for iq streaming using FT245 protocol.\r\n");

	return 0;
};

int deinit_stream() {

	printf("Close device  for streaming.\r\n");
	FT_STATUS ftStatus = FT_OK;
	ftStatus |= FT_SetBitMode(ftHandleA, 0, 0);
	ftStatus |= FT_Purge(ftHandleA, FT_PURGE_RX | FT_PURGE_TX);

	if (FT_Close(ftHandleA) != FT_OK) return -1;

	return 0;
}


int write_stream(unsigned char stream[]){
	
	FT_STATUS ftStatus = FT_OK;

	DWORD EventDWord;
	DWORD RxBytes;
	DWORD TxBytes;
	DWORD numBytesSent;


	ftStatus = FT_Write(ftHandleA, stream, 1032, &numBytesSent);


	if (ftStatus == FT_OK)
	{
		// FT_Read OK
		if (1032!= numBytesSent) {
			fprintf(stderr, "ds stream time not complete; only %d bytes sent \n", numBytesSent);
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "ds stream time out \n");
	}

	return 0;
	
}

int read_stream(unsigned char stream[]) {

	FT_STATUS ftStatus = FT_OK;

	DWORD EventDWord;
	DWORD RxBytes;
	DWORD TxBytes;
	DWORD BytesReceived;
	
	ftStatus = FT_Read(ftHandleA, stream, 1032, &BytesReceived);
			
	if (ftStatus == FT_OK)
	{
		if (1032 != BytesReceived) {
			fprintf(stderr, "us stream read not complete \n");
			return -1;
		} 
	}
	else
	{
		fprintf(stderr, "us stream time out \n");
		return -2;
	}
	
	return 0;
}

// End of source.,