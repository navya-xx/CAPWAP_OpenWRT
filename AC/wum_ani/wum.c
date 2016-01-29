#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include "wumLib.h"

void usage(char *name);
#define ACSERVER_ADDRESS "127.0.0.1"
#define ACSERVER_PORT	1235
/* parameters */
#define PARAM_NUM 3
/* to put message all message elements into a structure*/

enum {
NO_PARAMETER, TXPOWER, CHANNEL
};

//parameter_t PARAMs[] = { { TXPOWER, "power" }, { CHANNEL, "channel" }, { NO_PARAMETER, "" } };

int main(int argc, char *argv[]) {
	int acserver, wtpId, radioId;
	int parameter_id[2];
	int param_cnt = 0;
	int value[2];
	// Initialise a struct variable to put all message elements in a payload
	msg_elements msg_payload;
	msg_payload.offset=0;
	//WUM_INIT_REQ_MSG(msg_payload, 50);
	msg_elements msg;
	msg.offset=0;
	//WUM_INIT_REQ_MSG(msg, 50);//
	msg.payload = malloc(50);
	msg_payload.payload = malloc(50);
	char *acserver_address = ACSERVER_ADDRESS;
	int acserver_port = ACSERVER_PORT;
	int c;
	opterr = 0;

	/* Parse options */
	while ((c = getopt(argc, argv, "ha:p:w:c:v")) != -1)
		switch (c) {
		case 'a':
			acserver_address = optarg;
			break;
		case 'p':
			acserver_port = atoi(optarg);
			break;
		case 'w':
			wtpId = atoi(optarg);
			break;
		case 't':
			parameter_id[param_cnt] = TXPOWER ;
			value[param_cnt] = atoi(optarg);
			param_cnt++;
			break;
		case 'c':
			parameter_id[param_cnt] = CHANNEL;
			value[param_cnt] = atoi(optarg);
			param_cnt++;
			break;
		case 'h':
			usage(argv[0]);
			break;
		case '?':
			if (optopt == 'w' || optopt == 'c' || optopt == 't')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			exit(EXIT_FAILURE);
		default:
			usage(argv[0]);
			abort();

		}

	/* Check arguments
		if ((parameter == NULL) && (value == NULL)) {
			fprintf(stderr, "No parameter specified!\n");
			exit(EXIT_FAILURE);
		}

		if ((parameter_id = get_parameter_id(parameter)) == NO_PARAMETER) {
			fprintf(stderr, "Wrong parameter specified!");
		}*/
	/* Connect to server and get WTPs list */
	acserver = ACServerConnect(acserver_address, acserver_port);
		/* Execute command */
	int i = 0;
	for (i=0; i< param_cnt; i++){
		switch (parameter_id[i]) {
		case CHANNEL:
			set_channel(&msg_payload, value[i]);
			break;
		case TXPOWER:
			set_txpower(&msg_payload,value[i]);
			break;
		}
	}
	build_msg(&msg, &msg_payload, param_cnt,wtpId);
	WUMSendMessage(acserver, &msg);
	//WUM_DESTROY_MSG(msg);
	//WUM_DESTROY_MSG(msg_payload);
	ACServerDisconnect(acserver);

	exit(EXIT_SUCCESS);
}

/*int get_parameter_id(char *parameter) {
	int i;
	for (i = 0; i < PARAM_NUM; i++) {
		if (strcmp(PARAMETERs[i].name, parameter) == 0)
			break;
	}

	return PARAMETERs[i].id;
}*/

void usage(char *name) {
	printf(
			"%s -w id -c channel -t txpower [-a address] [-p port]\n", name);
	}
