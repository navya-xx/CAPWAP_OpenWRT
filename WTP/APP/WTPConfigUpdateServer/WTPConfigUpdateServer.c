/* Created by : Navneet Agrawal
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "WTPConfigUpdateServer.h"

int client_process(int);
unsigned char CWProtocolRetrieve8(CWProtocolMessage *msgPtr);
unsigned short CWProtocolRetrieve16(CWProtocolMessage *msgPtr);

int main(int argc, char *argv[]) {
	int sockfd, newsockfd, portno, clilen, pid;
	struct sockaddr_in serv_addr, cli_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket\n");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	//portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(CONF_UPDATE_SERVER_PORT);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("[ConfUpdateAPPSERVER]: Error on binding.");
		close(sockfd);
		exit(1);
	}
	listen(sockfd, 5);
	printf("Connection opened.\n");
	clilen = sizeof(cli_addr);

	while (1) {
		/* Accepting connection from the Client */
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		printf("Connection new.\n");
		if (newsockfd < 0) {
			perror("ERROR on accept");
			exit(1);
		} else {
			printf("Connected with client socket number %d\n", newsockfd);
		}
		/*Forking... */
		pid = fork();
		if (pid < 0) {
			perror("Error on fork!");
			exit(1);
		} else if (pid == 0) {
			/*client processes*/
			close(sockfd);
			while ((client_process(newsockfd) == 1)) {
			}
			perror("Client said bye! Finishing...");
			exit(0);
		} else
			close(newsockfd);

	}
	return 0;
}

int client_process(int client_socket) {
	char* buffer;
	buffer = (char*) malloc(MAX_UDP_PACKET_SIZE);

	printf("Entering client_process \n");
	//memset(buffer, 0, MAX_UDP_PACKET_SIZE);
	CWProtocolResultCode* getResults;
	getResults = (CWProtocolResultCode*) malloc(sizeof(CWProtocolResultCode));
	//CW_CREATE_OBJECT_ERR(getResults, CWProtocolResultCode, return 0;);
	int n, flag = 0, write_buffer = 1;
	/*if ((n = write(client_socket, htonl(write_buffer), 255)) < 0) {
		perror("[ConfUpdateAPPSERVER]: Error in writing to client.");
		close(client_socket);
		return 0;
	}*/

	if ((n = read(client_socket, buffer, MAX_UDP_PACKET_SIZE)) < 0) {
		perror("[ConfUpdateAPPSERVER]: Error in reading from client.");
		close(client_socket);
		return 0;
	} else {
		printf("number of bytes read %d\n", n);
	}

	// we received the Configuration Update Message
	if (!CWUpdateConfiguration_ParseBindingValues(buffer, n, &getResults)) {
		perror("[ConfUpdateAPPSERVER]: Error parsing the message elements.");
		return 0;
	}

	if ((n = write(client_socket, (char*) getResults, sizeof(CWProtocolResultCode))) < 0) {
		perror("[ConfUpdateAPPSERVER]: Error in writing to client.");
		close(client_socket);
		return 0;
	} else {
		printf("n bytes written on client %d",n);

	}

	free(buffer);

	return 1;
	//if (n < 0) error("ERROR writing to socket");
}


int CWUpdateConfiguration_ParseBindingValues(char* msg, int len, CWProtocolResultCode* result) {
	int i;
	CWProtocolMessage completeMsg;

	if (msg == NULL){
		perror("Message element not found!!");
		return 0;
	}

	printf("Parsing Binding Configuration Update Request...\n");

	completeMsg.msg = msg;
	completeMsg.offset = 0;

	// parse message elements
	while (completeMsg.offset < len) {
		unsigned short int elemType = 0;// = CWProtocolRetrieve32(&completeMsg);
		unsigned short int elemLen = 0;// = CWProtocolRetrieve16(&completeMsg);

		CWParseFormatMsgElem(&completeMsg, &elemType, &elemLen);

		//GlobalElemType = elemType;

		printf("Parsing Message Element: %u, elemLen: %u\n", elemType, elemLen);

		switch (elemType) {
		case BINDING_MSG_ELEMENT_TYPE_TXPOWER:
			if (!(CWParseTXPOWER(&completeMsg, elemLen))) {
							//CW_FREE_OBJECT(auxBindingPtr->radioQosValues);
							//CW_FREE_OBJECT(valuesPtr);
							return 0; // will be handled by the caller
						}
			//completeMsg.offset += elemLen;
			break;
		case BINDING_MSG_ELEMENT_TYPE_OFDM_CONTROL:
			if (!(CWParseWTPOFDM(&completeMsg, elemLen))) {
										//CW_FREE_OBJECT(auxBindingPtr->radioQosValues);
										//CW_FREE_OBJECT(valuesPtr);
										return 0; // will be handled by the caller
									}
			//completeMsg.offset += elemLen;
			break;
		default:
				printf("Message type not recognized! Moving to next message element...\n");
				completeMsg.offset += elemLen;
				break;
		}
	}

	if (completeMsg.offset != len){
		printf("Garbage at the end of the message!! completeMsg.offset %d, len %d", completeMsg.offset, len);
		return 1;
	}

	/*switch (GlobalElemType) {
	case BINDING_MSG_ELEMENT_TYPE_WTP_QOS: {
		CW_CREATE_OBJECT_ERR(auxBindingPtr, CWBindingConfigurationUpdateRequestValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY,NULL););
		*valuesPtr = (void *) auxBindingPtr;

		auxBindingPtr->qosCount = qosCount;
		auxBindingPtr->radioQosValues = NULL;

		CW_CREATE_ARRAY_ERR(auxBindingPtr->radioQosValues, auxBindingPtr->qosCount, RadioQosValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		break;
	}
	case BINDING_MSG_ELEMENT_TYPE_OFDM_CONTROL:
		CW_CREATE_OBJECT_ERR(ofdmBindingPtr, CWBindingConfigurationUpdateRequestValuesOFDM, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY,NULL););

		*valuesPtr = (void *) ofdmBindingPtr;

		CW_CREATE_OBJECT_ERR(ofdmBindingPtr->radioOFDMValues, OFDMControlValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY,NULL););
		break;
	}

	i = 0;
	completeMsg.offset = 0;
	while (completeMsg.offset < len) {
		unsigned short int type = 0;
		unsigned short int elemLen = 0;

		CWParseFormatMsgElem(&completeMsg, &type, &elemLen);

		switch (type) {
		case BINDING_MSG_ELEMENT_TYPE_TXPOWER: {
			unsigned char tagPackets;
			if (!(CWParseTXPOWER(&completeMsg, elemLen, &(auxBindingPtr->radioQosValues[i].radioID),
					&tagPackets, auxBindingPtr->radioQosValues[i].qosValues))) {
				CW_FREE_OBJECT(auxBindingPtr->radioQosValues);
				CW_FREE_OBJECT(valuesPtr);
				return CW_FALSE; // will be handled by the caller
			}
			i++;
			break;
		}
		case BINDING_MSG_ELEMENT_TYPE_OFDM_CONTROL:  2009: New case
		{
			if (!(CWParseWTPOFDM(&completeMsg, elemLen, &(ofdmBindingPtr->radioID),
					ofdmBindingPtr->radioOFDMValues))) {
				CW_FREE_OBJECT(ofdmBindingPtr->radioOFDMValues);
				CW_FREE_OBJECT(valuesPtr);
				return CW_FALSE; // will be handled by the caller
			}
			break;
		}
		default:
			completeMsg.offset += elemLen;
			break;
		}
	}*/

	printf("Binding Configure Update Request Parsed..\n");

	return 1;
}

int CWParseTXPOWER(CWProtocolMessage *completeMsg, int elemLen){
	// get txpower value
	// radio ID
	int radioID;
	radioID = (int) CWProtocolRetrieve8(completeMsg);

	//Skip reserved byte
	completeMsg->offset += 1;

	//tx power
	unsigned short txpower;
	txpower = CWProtocolRetrieve16(completeMsg);

	//now use UCI command to set TxPower
	if(!CWSetTxPower((int) txpower)){
		perror("unable to set TxPower!");
		return 0;
	}
	return 1;
}

int CWSetTxPower(int txpower) {
	char* txp;
	sprintf(txp, "%d", txpower);
	// uci command
	char cmd[100];
	strcpy(cmd, "uci set wireless.@wifi-device[0].txpower=");
	strcat(cmd, txp);
	strcat(cmd, "; uci commit wireless; wifi");
	SYSTEM_RUN(cmd);
	return 1;
}

int CWParseWTPOFDM(CWProtocolMessage *completeMsg, int elemLen){
	//get channel value
	// radio ID
	int radioID;
	radioID = (int) CWProtocolRetrieve8(completeMsg);

	//Skip reserved byte
		completeMsg->offset += 1;

	//current chan
	int channel;
	channel = (int) CWProtocolRetrieve8(completeMsg);
	printf("Channel retrieved %d\n", channel);

	//now use UCI command to set channel
	if(CWSetChan(channel) != 1){
		printf("unable to set Channel!");
		return 0;
	}

	//band support + TIThreshhold - Ignore
	completeMsg->offset += 5;


		return 1;

}

int CWSetChan(int channel) {
	/*char* chan;
	sprintf(chan, "%d", channel);
	// uci command
	char cmd[100];
	strcpy(cmd, "uci set wireless.@wifi-device[0].channel=");
	strcat(cmd, chan);
	strcat(cmd, "; uci commit wireless; wifi");
	strcat(cmd, ";echo \"test\"");*/

	printf("Command channel set cmd \n");

	//SYSTEM_RUN(cmd);
	return 1;
}

int CWParseFormatMsgElem(CWProtocolMessage *completeMsg,
		unsigned short int *type, unsigned short int *len) {
	*type = CWProtocolRetrieve16(completeMsg);
	*len = CWProtocolRetrieve16(completeMsg);
	return 1;
}

// retrieves 8 bits from the message, increments the current offset in bytes.
unsigned char CWProtocolRetrieve8(CWProtocolMessage *msgPtr) {
	unsigned char val;

	CW_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 1);
	(msgPtr->offset) += 1;

	return val;
}

// retrieves 16 bits from the message, increments the current offset in bytes.
unsigned short CWProtocolRetrieve16(CWProtocolMessage *msgPtr) {
	unsigned short val;

	CW_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 2);
	(msgPtr->offset) += 2;

	return ntohs(val);
}

// retrieves 32 bits from the message, increments the current offset in bytes.
unsigned int CWProtocolRetrieve32(CWProtocolMessage *msgPtr) {
	unsigned int val;

	CW_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 4);
	(msgPtr->offset) += 4;

	return ntohl(val);
}

// retrieves a string (not null-terminated) from the message, increments the current offset in bytes.
// Adds the '\0' char at the end of the string which is returned
char *CWProtocolRetrieveStr(CWProtocolMessage *msgPtr, int len) {
	char *str;

	CW_CREATE_OBJECT_SIZE_ERR(str, (len+1), return NULL;);

	CW_COPY_MEMORY(str, &((msgPtr->msg)[(msgPtr->offset)]), len);
	str[len] = '\0';
	(msgPtr->offset) += len;

	return str;
}

// retrieves len bytes from the message, increments the current offset in bytes.
char *CWProtocolRetrieveRawBytes(CWProtocolMessage *msgPtr, int len) {
	char *bytes;

	CW_CREATE_OBJECT_SIZE_ERR(bytes, len, return NULL;);

	CW_COPY_MEMORY(bytes, &((msgPtr->msg)[(msgPtr->offset)]), len);
	(msgPtr->offset) += len;

	return bytes;
}
