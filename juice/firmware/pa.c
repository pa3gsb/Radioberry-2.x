#include "pa.h" 

int pa_version;
uint16_t pa_temp;
bool pa_temp_ok = true;
uint16_t pa_fwd_dac;
uint16_t pa_rev_dac;
int pa_err;

int pa_client_socket = -1;
unsigned char pa_protocol_header[4] = {0x00, 0x00, 0xEF, 0xFE};

char lpf = 0x00;
char ptt = 0x00;
char state = 0x00;

char message[20] = {};
sem_t mutex;

int client_connected = -1;

int currentfreq;

void initAmplifier(void) {
	memset(message, 0, 20);
	memcpy(message, pa_protocol_header, 4);
	
	sem_init(&mutex, 0, 1);	
}

int determine_freq(int base_index, unsigned char* buffer) {
	return (((buffer[base_index + 1] & 0xFF) << 24) + ((buffer[base_index + 2] & 0xFF) << 16) + ((buffer[base_index + 3] & 0xFF) << 8) + (buffer[base_index + 4] & 0xFF));
}

void handleLPFs(unsigned char* buffer) {
	if ((buffer[11] & 0xFE) == 0x04) {
		currentfreq = determine_freq(11, buffer);
	};
	if ((buffer[523] & 0xFE) == 0x04) {
		currentfreq = determine_freq(523, buffer);
	};
	
	// firmware always determines the LPF switch.
	// need to think about the frequencies!!!!! todo
	if (currentfreq > 15000000) lpf = 0x04;     	/* 10 meters */
		else if (currentfreq > 8000000) lpf = 0x08; /* 20 meters */
		else if (currentfreq > 6000000) lpf = 0x10; /* 40 meters */
		else if (currentfreq > 4000000) lpf = 0x20; /* 60 meters */
		else if (currentfreq > 3000000) lpf = 0x40; /* 80 meters */
		else if (currentfreq > 1000000) lpf = 0x80; /* 160 meters */
		else lpf = 0x00; 
}

void setAmplifierInfo(void){
	sem_wait(&mutex);
	memcpy(message, pa_protocol_header, 4);
	message[4] = 0x0F;
	message[5] = 0x00;
	sem_post(&mutex); 
}

void setAmplifierInfoBuffer(unsigned char* buffer){
	sem_wait(&mutex);
	memcpy(message, pa_protocol_header, 4);
	message[4] = ((buffer[3] & 0xFF)) == 0x00 ? 0x0F : 0xFF;
	message[5] = ((buffer[11] & 0x01)==0x01) ? 0xFF:0x00;
	handleLPFs(buffer);
	message[6] = lpf;
	sem_post(&mutex); 
}

int connectToAmplifier() {
    struct sockaddr_in server_address;
    char server_ip[] = "169.254.19.101";
    int server_port = 4242;

    if (client_connected == 2) return 0;

    pa_client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (pa_client_socket < 0) {
        fprintf(stderr, "PA Client Socket creation failed\n");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address / Address not supported\n");
#ifdef _WIN32
        closesocket(pa_client_socket);
#else
        close(pa_client_socket);
#endif
        return -1;
    }

#ifdef _WIN32
    int wtimeout = 100;
    if (setsockopt(pa_client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&wtimeout, sizeof(int)) < 0)
        perror("setsockopt failed\n");
    if (setsockopt(pa_client_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&wtimeout, sizeof(int)) < 0)
        perror("setsockopt failed\n");
#else
    int optval;
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(pa_client_socket, SOL_SOCKET, SO_RCVTIMEO, (const void*)&timeout, sizeof(timeout)) < 0)
        perror("setsockopt: SO_RCVTIMEO");

    optval = 7; // high priority
    if (setsockopt(pa_client_socket, SOL_SOCKET, SO_PRIORITY, &optval, sizeof(optval)) < 0) {
        perror("setsockopt: SO_PRIORITY");
    }
#endif

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(pa_client_socket, FIONBIO, &mode);
#else
    int flags = fcntl(pa_client_socket, F_GETFL, 0);
    fcntl(pa_client_socket, F_SETFL, flags | O_NONBLOCK);
#endif

    client_connected = connect(pa_client_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (client_connected == -1) {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            WSAEVENT connectEvent = WSACreateEvent();
            WSAEventSelect(pa_client_socket, connectEvent, FD_CONNECT);

            DWORD waitResult = WSAWaitForMultipleEvents(1, &connectEvent, TRUE, 10000, FALSE);
            if (waitResult == WSA_WAIT_TIMEOUT) {
                fprintf(stderr, "AMP Connection timed out\n");
            } else if (waitResult == WSA_WAIT_FAILED) {
                perror("WSAWaitForMultipleEvents failed");
            } else if (waitResult == WSA_WAIT_EVENT_0) {
                int error = 0;
                int len = sizeof(error);
                if (getsockopt(pa_client_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len) == 0 && error == 0) {
                    client_connected = 2; // Success
                    fprintf(stderr, "AMP Connected\n");
                } else {
                    fprintf(stderr, "AMP Connection failed: %d\n", error);
					//Normally Network is unreachable.
					client_connected = -1;
                }
            }
            WSACloseEvent(connectEvent);
        }
#else
        if (errno == EINPROGRESS) {
            fd_set writefds;
            FD_ZERO(&writefds);
            FD_SET(pa_client_socket, &writefds);

            int selectResult = select(pa_client_socket + 1, NULL, &writefds, NULL, &timeout);

            if (selectResult == 0) {
                fprintf(stderr, "AMP Connection timed out\n");
            } else if (selectResult == -1) {
                perror("select failed");
            } else if (FD_ISSET(pa_client_socket, &writefds)) {
                int error = 0;
                socklen_t len = sizeof(error);
                if (getsockopt(pa_client_socket, SOL_SOCKET, SO_ERROR, &error, &len) == 0) {
                    if (error == 0) {
                        client_connected = 2; // Success
                        fprintf(stderr, "AMP Connected\n");
                    } else {
                        fprintf(stderr, "AMP Connection failed: %s\n", strerror(error));
						client_connected = -1;
                    }
                } else {
                    perror("getsockopt");
                }
            }
        }
#endif
    } else {
        fprintf(stderr, "AMP Connection established immediately, checking status...\n");
        int error = 0;
        socklen_t len = sizeof(error);
#ifdef _WIN32
        if (getsockopt(pa_client_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len) == 0 && error == 0) {
#else
        if (getsockopt(pa_client_socket, SOL_SOCKET, SO_ERROR, &error, &len) == 0 && error == 0) {
#endif
            client_connected = 2; // Success
            fprintf(stderr, "AMP Connected\n");
        } else {
            fprintf(stderr, "AMP Connection failed: %s\n", strerror(error));
			client_connected = -1;
        }
    }

    return client_connected;
}


int isAmplifierConnected(){
	if (client_connected >= 0) return 1;
	return 0;
}

void disconnectAmplifier() {
#ifdef _WIN32
	if (pa_client_socket >= 0) {
		closesocket(pa_client_socket);
	} 
#else
	if (pa_client_socket >= 0) close(pa_client_socket);
#endif
	pa_client_socket = -1;
	client_connected = -1;
}

void inform_PA() {
	sem_wait(&mutex); 
	int bytesSent = send(pa_client_socket, message, sizeof(message), 0);
	if (bytesSent <= 0) disconnectAmplifier();
	sem_post(&mutex);
}

void inform_user() {
    char buffer[20];
	memset(buffer, 0, 20);

	int bytesRead = recv(pa_client_socket, buffer, sizeof(buffer), 0); 
    
	uint32_t code;
	memcpy(&code, buffer, 4);
	switch (code)
	{
		default:
			// no action.
			break;
		case 0xfeef0000:
			{
				pa_version = buffer[4];	
				pa_temp_ok = (buffer[5] == 0xFF) ? false : true;			
				pa_temp =    (uint16_t) (buffer[6] & 0xFF);
				pa_fwd_dac = (uint16_t)(((buffer[7] & 0x0F) << 8) | (buffer[8] & 0xFF));
				pa_rev_dac = (uint16_t)(((buffer[9] & 0x0F) << 8) | (buffer[10] & 0xFF));
				pa_err  =  buffer[19];
			}
			break;
	}
}

void exhangeAmplifierInfo() {
	if (isAmplifierConnected()) {
		inform_PA();
		inform_user();
	}
}

// end of source.