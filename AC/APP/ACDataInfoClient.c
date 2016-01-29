#include "ACDataInfoClient.h"
void* print_station_info(struct sta_info station);

int create_data_Frame(CWProtocolMessage** frame, char* buffer, int len) {

	CW_CREATE_OBJECT_ERR(*frame, CWProtocolMessage, return 0;);
	CWProtocolMessage *auxPtr = *frame;
	CW_CREATE_PROTOCOL_MESSAGE(*auxPtr, len, return 0;);
	memcpy(auxPtr->msg, buffer, len);
	auxPtr->offset = len;
	return 1;
}

int parseDataStats(CWProtocolMessage *msg) {
	int stationCount = 0, i;
	struct sta_info station;
	int size_sta = sizeof(struct sta_info);
	bcopy((char*) (msg->msg + 4), (char*) &stationCount, 4);
	stationCount = ntohl(stationCount);
	for (i = 0; i < stationCount; i++) {
		bcopy((char*) (msg->msg + 8 + (i * size_sta)), (char*) &station, size_sta);
		print_station_info(station);
	}
	return 1;
}

void* print_station_info(struct sta_info station) {
	// mac addr
	char mac[17];
	bcopy((char*) &station, &mac, 17);
	printf("MAC ADDR: %s\n", mac);
}

int main(int argc, char *argv[]) {
	int sock, rlen, len, fromlen;
	struct sockaddr_un servaddr;
	struct sockaddr_un from;
	CWProtocolMessage* data = NULL;
	int wtpIndex;
	wtpIndex = atoi(argv[1]);
	static char buffer[MAX_INPUT_SIZE + 1];
	char sockserv_path[50];
	char string[10];

	//make unix socket client path name by index i
	snprintf(string, sizeof(string), "%d", wtpIndex);
	string[sizeof(string) - 1] = 0;

	strcpy(sockserv_path, SOCKET_PATH_RECV_AGENT);
	strcat(sockserv_path, string);

	printf("socket server path %s\n", sockserv_path);

	strcpy(servaddr.sun_path, sockserv_path);

	/*      Create a UNIX datagram socket for this thread        */
	if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		perror("AC data info client: Error creating socket");
		return 0;
	}

	/*      Set up address structure for server socket      */
	bzero(&servaddr, sizeof(servaddr));
	bzero(&from, sizeof(from));
	servaddr.sun_family = AF_UNIX;
	//strcpy(servaddr.sun_path, SOCKET_PATH_RECV_AGENT);

	unlink(sockserv_path);

	len = sizeof(servaddr.sun_family) + strlen(servaddr.sun_path);

	if (bind(sock, (const struct sockaddr *) &servaddr, len) < 0) {
		perror("THR STATS: Error binding socket");
		return 0;
	} else
		printf("bind on socket\n");

	fromlen = sizeof(from);

	/*      Receive data */
	while (1) {
		rlen = recvfrom(sock, buffer, MAX_INPUT_SIZE, 0, (struct sockaddr *) &from,
				(socklen_t *) &fromlen);
		printf("received %d bytes\n", rlen);
		if (!create_data_Frame(&data, buffer, rlen)) {
			perror("Error extracting a data stats frame");
			return 0;
		}

		if (!parseDataStats(data)) {
			printf("Failed to parse data stats (station info)\n");
			return 0;
		}

	}
	return 1;
}
