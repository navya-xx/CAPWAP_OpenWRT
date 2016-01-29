/*
 * Created by Navneet
 */

#include "netlinkTest.h"

int client_process(char* data, int* len) {
	// Get station dump
	bzero(data, MAX_INPUT_SIZE);
	if ((*len = get_station_dump(data)) < 0) {
		perror("error!!");
		return -1;
	} else if (*len == 0) {
		printf("Size of len = %d\n", *len);
		return 0;
	} else
		printf("message len: %d\n", *len);
	return 1;
}

int get_station_dump(char* msg_sta) {
	struct nl80211_state state;
	int err;

	int devidx;
	err = nl80211_init(&state);
	if (err < 0) {
		printf("error!!");
	} else
		printf("Initiated nl80211 state machine.\n");

	struct nl_msg *msg;
	msg = nlmsg_alloc();
	printf("Allocated nl msg.\n");

	if (!msg) {
		printf("failed to allocate netlink message\n");
		return -1;
	}
	devidx = if_nametoindex("wlan0");

	//calling the station dump function
	struct sta_info* out_test;
	out_test = (struct sta_info*) malloc(MAX_STATIONS * sizeof(struct sta_info));
	memset(out_test, 0, MAX_STATIONS * sizeof(struct sta_info));
	printf("Created variable for storing station info for 10 stations MAX.\n");

	unsigned int station_count = 0;
	err = station_dump(msg, &state, devidx, out_test, (int*) &station_count);
	printf("Received station dump. station count: %d\n", station_count);
	printf("station info received.. mac addr: %s\n", out_test->mac_addr);

	int info_size = station_count * (sizeof(struct sta_info));
	printf("info size %d\n", info_size);
	if (info_size == 0)
		return 0;

	station_count = htonl(station_count);
	//copy total size of message
	int totalSize = info_size + 2*sizeof(unsigned int);
	totalSize = htonl(totalSize);
	bcopy((char*) &totalSize, msg_sta, sizeof(unsigned int));
	// copy station count to msg
	bcopy((char*) &station_count, (msg_sta + 4), sizeof(unsigned int));
	// copy station info to msg
	bcopy((char*) out_test, (msg_sta + 8), info_size);
	free(out_test);
	out_test = NULL;

	printf("stored information in a message.\n");
	return info_size + 2*sizeof(unsigned int);

}

int main(int argc, char *argv[]) {

	char stadata[MAX_INPUT_SIZE];
	int sock, socklen = 0, slen=0, len = 0;

	struct sockaddr_un servaddr;

	if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		perror("Netlink Server Unix: Error creating socket");
		return 0;
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path, SOCKET_PATH);

	//unlink(SOCKET_PATH);

	socklen = sizeof(servaddr.sun_family) + strlen(servaddr.sun_path);

	/*if (bind(sock, (const struct sockaddr *) &servaddr, socklen) < 0) {
		perror("Netlink server: Error binding socket");
		return 0;
	}*/

	while (1) {
		/*
		 * Receiving information regarding connected stations
		 * -----------------------------------------------------
		 */
		int client_process_out = client_process(stadata, &len);
		if (client_process_out < 0) {
			perror("error in reading station info.\n");
			return 0;
		} else if (client_process_out == 0) {
			perror("no data to write.\n");
			sleep(STAINFO_SLEEP_TIME);
			continue;
		} else {
			printf("data %s, len %d\n", stadata, len);
		}
		/*
		 * -----------------------------------------------------
		 *
		 */

		if (slen = sendto(sock, stadata, len, 0, &servaddr, sizeof(struct sockaddr_un)) < 0) {
			perror("[STA INFO DATA SEND]:Failed to send data.");
			return 0;
		} else {
			perror("[STA INFO DATA SEND]:Successfully sent data.");
		}
		sleep(STAINFO_SLEEP_TIME);
	}

	return 1;

}
