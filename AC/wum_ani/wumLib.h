/*
 Documentation for message structure and message elemnt details
 Message:
 1byte = 2;
 4 bytes = wtpIndex int
 4 bytes = messagelemCount
 4 bytes = messageLength
 message elements

message element types:

1. OFDM control to change the channel (6.10 in RFC 5416)---- but this binding is only for IEEE 802.11a
message element structure
Radio ID = 1byte
Reserve = 1byte
Current channel = 1byte
Band support = 1byte
TI threshold = 4bytes
There is also another frequency changing message element for 802.11b (RFC 5416 6.5)

2. Tx power (6.18 RFC 5416)
Radio ID = 1byte
Reserved = 1byte
currentTxPower = 2bytes
Power should be in mW.
*/
#include <stdio.h>

#define SUCCESS	0
#define ERROR	1
#define OFDM_CONTROL_TYPE 1033
#define OFDM_CONTROL_LEN 8
#define TXPOWER_CONTROL_TYPE 1041
#define TXPOWER_CONTROL_LEN 4

//#define WUM_INIT_REQ_MSG(msg, size) do { msg->offset = 0; msg->payload=malloc(size); } while(0);
//#define WUM_DESTROY_MSG(msg)  do { if (msg->offset != 0) free(msg->payload); } while(0);


typedef struct{
	int offset;
	char *payload;
}msg_elements;

int ACServerConnect(char *address, int port);
void ACServerDisconnect(int acserver);
int Readn(int fd, void *ptr, size_t nbytes);
int Writen(int fd, void *ptr, size_t nbytes);
int Read32(int fd, int *ptr);
int Write32(int fd, void *ptr);
void WUMPayloadStore8(msg_elements *msg_payload, char c);
void WUMPayloadStore32(msg_elements *msg_payload, int i);
void WUMPayloadStore16(msg_elements *msg_payload, short int i);
void WUMPayloadStoreRawBytes(msg_elements *msg, msg_elements *msg_payload);
void set_channel(msg_elements *msg_payload, int value);
void set_txpower(msg_elements *msg_payload, int value);
void build_msg(msg_elements *msg, msg_elements *msg_payload, int param_cnt, int wtp_id);

