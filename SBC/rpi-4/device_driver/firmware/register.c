#include "register.h"


#define SA      struct sockaddr
#define MAXLINE 4096
#define MAXSUB  512

PROPERTY* properties=NULL;

int getMacAddress(void) {
	
	struct ifaddrs *ifaddr=NULL;
    struct ifaddrs *ifa = NULL;
	int found = 0;

    if (getifaddrs(&ifaddr) != -1) {
         for ( ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
             if ( (ifa->ifa_addr) && (ifa->ifa_addr->sa_family == AF_PACKET) && !(ifa->ifa_flags & IFF_LOOPBACK) ) {
                  struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
                  
				  if ((strstr(ifa->ifa_name, "eth") != NULL) || (strstr(ifa->ifa_name, "wlan") != NULL) ) {
					 char *macstr = "%02X:%02X:%02X:%02X:%02X:%02X";
					 sprintf(macaddress,macstr, s->sll_addr[0], s->sll_addr[1], s->sll_addr[2], s->sll_addr[3], s->sll_addr[4], s->sll_addr[5]);
					 found  = 1;
					 break;
				  }
             }
         }
	}
    freeifaddrs(ifaddr);
	
	return found;
}

void loadProperties(char* filename) {
    char string[80];
    char* name;
    char* value;
    FILE* f=fopen(filename,"r");
    PROPERTY* property;
    if(f) {
        while(fgets(string,sizeof(string),f)) {
            if(string[0]!='#') {
                name=strtok(string,"=");
                value=strtok(NULL,"\n");
				// Beware of "illegal" lines in corrupted files
				if (name != NULL && value != NULL) {
					  property=malloc(sizeof(PROPERTY));
					  property->name=malloc(strlen(name)+1);
					  strcpy(property->name,name);
					  property->value=malloc(strlen(value)+1);
					  strcpy(property->value,value);
					  property->next_property=properties;
					  properties=property;
				}
            }
        }
        fclose(f);
    }
}

char* getProperty(char* name) {
    char* value=NULL;
    PROPERTY* property=properties;
    while(property) {
        if(strcmp(name,property->name)==0) {
            value=property->value;
            break;
        }
        property=property->next_property;
    }
    return value;
}

void loadRadioberryProps() {
	
	loadProperties("/home/pi/.radioberry/radioberry.props");
	
	radiocall = getProperty("call");
	if (radiocall==NULL) radiocall="-";
	if (strlen(radiocall) > 16) radiocall[16]  = '\0';
	radiolocator = getProperty("locator");
	if (radiolocator==NULL) radiolocator="-";
	if (strlen(radiolocator) > 6) radiolocator[6]  = '\0';
}

ssize_t process_http(int sockfd, char *host, char *page, char *poststr)
{
	char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
	ssize_t n;
	snprintf(sendline, MAXSUB,
		 "POST %s HTTP/1.0\r\n"
		 "Host: %s\r\n"
		 "Content-type: application/x-www-form-urlencoded\r\n"
		 "Content-length: %d\r\n\r\n"
		 "%s", page, host, strlen(poststr), poststr);

	write(sockfd, sendline, strlen(sendline));
	//get response
	while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
		recvline[n] = '\0';
		//printf("%s", recvline);
	}
	return n;
}

void postRadioberryConfiguration() {
	int sockfd;
	struct sockaddr_in servaddr;

	char **pptr;
	char *hname = "www.pa3gsb.nl";
	char *page = "/radioberry/api/write.php";
	char *poststr = "macaddress=%s&radiocall=%s&radiolocator=%s&gatewareversion=%s&driverversion=%s&firmwareversion=%s&fpgatype=%s\r\n";

	// make message for registration.
	char post_message[4096];
	sprintf(post_message,poststr, macaddress, radiocall, radiolocator, gatewareversion, driverversion, firmwareversion, fpgatype);

	char str[50];
	struct hostent *hptr;
	if ((hptr = gethostbyname(hname)) == NULL) {
		return;
	}
	if (hptr->h_addrtype == AF_INET && (pptr = hptr->h_addr_list) != NULL) inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(80);
	inet_pton(AF_INET, str, &servaddr.sin_addr);
	connect(sockfd, (SA *) & servaddr, sizeof(servaddr));
	process_http(sockfd, hname, page, post_message);
	
	close(sockfd);
}

void registerRadioberry() {
	if (!getMacAddress()) return;
	loadRadioberryProps();
	postRadioberryConfiguration();
}

//end of source