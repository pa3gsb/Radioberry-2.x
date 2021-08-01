#include "register.h"


#define SA      struct sockaddr
#define MAXLINE 4096
#define MAXSUB  512

PROPERTY* properties=NULL;

#ifdef _WIN32

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define IP_LOCALHOST    0x0100007F
 
int getMacAddress(void)
{
   int found = 0;
   ULONG nSize = 0;
   MIB_IPADDRTABLE * ip_table = NULL;
   if (GetIpAddrTable(ip_table, &nSize, 0) == ERROR_INSUFFICIENT_BUFFER)
   {
       ip_table = (MIB_IPADDRTABLE *) malloc(nSize);
       if (GetIpAddrTable(ip_table, &nSize, 0) == NO_ERROR)
       {
           for (int i = 0; i < ip_table->dwNumEntries; ++i)
           {
               if (IP_LOCALHOST != ip_table->table[i].dwAddr)
               {
                   MIB_IFROW iInfo;
                   BYTE byMAC[6] = { 0, 0, 0, 0, 0, 0 };
                   memset(&iInfo, 0, sizeof(MIB_IFROW));
                   iInfo.dwIndex = ip_table->table[i].dwIndex;
                   GetIfEntry(&iInfo);
				   if (IF_TYPE_IEEE80211 == iInfo.dwType) {
                        memcpy(&byMAC, iInfo.bPhysAddr, iInfo.dwPhysAddrLen);
						char *macstr = "%02X:%02X:%02X:%02X:%02X:%02X";
						sprintf(macaddress,macstr, byMAC[0],byMAC[1],byMAC[2], byMAC[3],byMAC[4],byMAC[5]);
						//printf("%s", macaddress);
						found  = 1;
					    break;
                   }
               }
           }
       }
       free(ip_table);
   }
   return found;
 }

#else

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

#endif

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
					  property=(PROPERTY *)malloc(sizeof(PROPERTY));
					  property->name=(char *)malloc(strlen(name)+1);
					  strcpy(property->name,name);
					  property->value=(char *)malloc(strlen(value)+1);
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
	#ifndef _WIN32
		loadProperties("/home/pi/.radioberry/radioberry.props");
	#else
		loadProperties("./radioberry.props");
	#endif
	
	radiocall = getProperty("call");
	if (radiocall==NULL) radiocall="-";
	if (strlen(radiocall) > 16) radiocall[16]  = '\0';
	radiolocator = getProperty("locator");
	if (radiolocator==NULL) radiolocator="-";
	if (strlen(radiolocator) > 6) radiolocator[6]  = '\0';
	//fprintf(stderr, "call = %s, locator = %s", radiocall, radiolocator); 
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
	send(sockfd, sendline, strlen(sendline), 0);
	//get response
	while ((n = recv(sockfd, recvline, MAXLINE, 0)) > 0) {
		recvline[n] = '\0';
		//printf("%s", recvline);
	}
	return n;
}

int postRadioberryConfiguration() {
	
	int result = 0;
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
	if ((hptr = gethostbyname(hname)) == NULL) return -1;
	if (hptr->h_addrtype == AF_INET && (pptr = hptr->h_addr_list) != NULL) inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(80);
	inet_pton(AF_INET, str, &servaddr.sin_addr);
	if (connect(sockfd, (SA *) & servaddr, sizeof(servaddr)) >= 0) 	process_http(sockfd, hname, page, post_message); else result = -1;
	
	#ifdef _WIN32
		closesocket(sockfd);
	#else
		if (sockfd != 0) close(sockfd);
	#endif
	
	return result;
}

void registerRadioberry() {
	if (!getMacAddress()) return;
	loadRadioberryProps();
	if (postRadioberryConfiguration()) fprintf(stderr, "Registering your Radioberry not registered.\n"); else fprintf(stderr, "Your Radioberry is registered: http://www.pa3gsb.nl/radioberry/api/read.php\n");
}
//end of source