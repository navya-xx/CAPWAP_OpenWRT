#define CONF_UPDATE_SERVER_PORT 1301
#define MAX_UDP_PACKET_SIZE 1024
#define BINDING_MSG_ELEMENT_TYPE_OFDM_CONTROL	1033
#define BINDING_MSG_ELEMENT_TYPE_TXPOWER	1041
#define CW_CREATE_OBJECT_ERR(obj_name, obj_type, on_err)	{obj_name = (obj_type*) (malloc(sizeof(obj_type))); if(!(obj_name)) {on_err}}
#define CW_CREATE_OBJECT_SIZE_ERR(obj_name, obj_size,on_err)	{obj_name = (malloc(obj_size)); if(!(obj_name)) {on_err}}
#define		CW_COPY_MEMORY(dst, src, len)			bcopy(src, dst, len)
typedef enum {
	CW_PROTOCOL_SUCCESS				= 0, //	Success
	CW_PROTOCOL_FAILURE_AC_LIST			= 1, // AC List message MUST be present
	CW_PROTOCOL_SUCCESS_NAT				= 2, // NAT detected
	CW_PROTOCOL_FAILURE				= 3, // unspecified
	CW_PROTOCOL_FAILURE_RES_DEPLETION		= 4, // Resource Depletion
	CW_PROTOCOL_FAILURE_UNKNOWN_SRC			= 5, // Unknown Source
	CW_PROTOCOL_FAILURE_INCORRECT_DATA		= 6, // Incorrect Data
	CW_PROTOCOL_FAILURE_ID_IN_USE			= 7, // Session ID Alreadyin Use
	CW_PROTOCOL_FAILURE_WTP_HW_UNSUPP		= 8, // WTP Hardware not supported
	CW_PROTOCOL_FAILURE_BINDING_UNSUPP		= 9, // Binding not supported
	CW_PROTOCOL_FAILURE_UNABLE_TO_RESET		= 10, // Unable to reset
	CW_PROTOCOL_FAILURE_FIRM_WRT_ERROR		= 11, // Firmware write error
	CW_PROTOCOL_FAILURE_SERVICE_PROVIDED_ANYHOW	= 12, // Unable to apply requested configuration
	CW_PROTOCOL_FAILURE_SERVICE_NOT_PROVIDED	= 13, // Unable to apply requested configuration
	CW_PROTOCOL_FAILURE_INVALID_CHECKSUM		= 14, // Image Data Error: invalid checksum
	CW_PROTOCOL_FAILURE_INVALID_DATA_LEN		= 15, // Image Data Error: invalid data length
	CW_PROTOCOL_FAILURE_OTHER_ERROR			= 16, // Image Data Error: other error
	CW_PROTOCOL_FAILURE_IMAGE_ALREADY_PRESENT	= 17, // Image Data Error: image already present
	CW_PROTOCOL_FAILURE_INVALID_STATE		= 18, // Message unexpected: invalid in current state
	CW_PROTOCOL_FAILURE_UNRECOGNIZED_REQ		= 19, // Message unexpected: unrecognized request
	CW_PROTOCOL_FAILURE_MISSING_MSG_ELEM		= 20, // Failure: missing mandatory message element
	CW_PROTOCOL_FAILURE_UNRECOGNIZED_MSG_ELEM	= 21  // Failure: unrecognized message element


} CWProtocolResultCode;

typedef struct {
	char *msg;
	int offset;
	int data_msgType;
} CWProtocolMessage;

/* Execute the system() function
 * and return CW_FALSE in case of failure. */
#define SYSTEM_RUN(cmd) \
do {  \
    int exit_c = system(cmd);  \
    if (!WIFEXITED(exit_c) || WEXITSTATUS(exit_c) != 0 ) { \
       return 0; \
    } \
} while(0)
