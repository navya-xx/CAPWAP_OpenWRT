/*******************************************************************************************
 * Copyright (c) 2009 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica   *
 *                      Universita' Campus BioMedico - Italy                               *
 *                                                                                         *
 * This program is free software; you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License as published by the Free Software Foundation; either  *
 * version 2 of the License, or (at your option) any later version.                        *
 *                                                                                         *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY         *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A         *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.                *
 *                                                                                         *
 * You should have received a copy of the GNU General Public License along with this       *
 * program; if not, write to the:                                                          *
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,                    *
 * MA  02111-1307, USA.                                                                    *
 *                                                                                         *
 * --------------------------------------------------------------------------------------- *
 * Project:  Capwap                                                                        *
 *                                                                                         *
 * Author : Navneet Agrawal (navneet.agrawal@iitb.ac.in)                                   *
 *                                            				                    		   *
 *******************************************************************************************/

#include "WTPStaInfoReceive.h"

int shm_object_open(CWWTP_msg_s** shared_msg) {
	int shm_fd;
	int shared_seg_size = (1 * sizeof(CWWTP_msg_s)); /* want shared segment capable of storing 1 message */
	/* the shared segment, and head of the messages list */

	/* creating the shared memory object    --  shm_open()  */
	if (!(shm_fd = shm_open(WTP_STAINFO_SHARED_MEMORY_PATH, O_CREAT | O_EXCL | O_RDWR,
			S_IRWXU | S_IRWXG))) {
		CWLog("[CLIENT APP_NetlinkServer]: Error in shm_open()");
		return -1;
	}
	if (shm_fd < 0) {
		CWLog("[CLIENT APP_NetlinkServer]: Error In shm_open()");
		return -1;
	}
	CWDebugLog("[CLIENT APP_NetlinkServer]: Created shared memory object %s\n",
			WTP_STAINFO_SHARED_MEMORY_PATH);

	/* requesting the shared segment    --  mmap() */
	*shared_msg = (CWWTP_msg_s*) mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (*shared_msg == NULL) {
		CWLog("[CLIENT APP_NetlinkServer]: Error In mmap()");
		return -1;
	}

	return shm_fd;
}

CW_THREAD_RETURN_TYPE CWWTPReceiveStaInfoStats(void *arg) {
	CWProtocolMessage *completeMsgPtr = NULL;
	CWProtocolMessage *data = NULL;
	CWBindingTransportHeaderValues *bindingValuesPtr = NULL;
	int shm_fd = 0, k, rlen, fragmentsNum = 0;
	char* buffer;
	CWWTP_msg_s* shared_msg = NULL;
	int shared_seg_size = (1 * sizeof(CWWTP_msg_s)); /* want shared segment capable of storing 1 message */

	while (1) /* Receive data Loop */
	{
		if ((shm_fd = shm_object_open(&shared_msg)) < 0) {
			CWLog("[CLIENT APP_NetlinkServer]: Error in opening Shared memory.");
			//CWExitThread();
			printf("[CLIENT APP_NetlinkServer]: error in opening shared memory.");
		}
		unsigned int len_buffer = 0;
		memcpy((char*) &len_buffer, shared_msg->content, 4);
		len_buffer = ntohl(len_buffer);
		rlen = 0;
		if (len_buffer > 0) {
			buffer = malloc(len_buffer + 4);
			memset(buffer, 0, len_buffer + 4);

			memcpy(buffer, shared_msg->content, len_buffer + 4);

			CW_CREATE_OBJECT_ERR(data, CWProtocolMessage, return 0;);
			CW_CREATE_PROTOCOL_MESSAGE(*data, rlen, return 0;);

			memcpy(data->msg, buffer, rlen);
			data->offset = rlen;

			/**************************************************************
			 * 2015 Update:                                               *
			 *                                                            *
			 * Station Info Message, like the QoS Data message are        *
			 * encapsuled on Capwap Data Message.                         *
			 * For distinguish the three types of message we use the      *
			 * fields of binding dataRate and SNR.                        *
			 * Station Info Message: dataRate=-1 && SNR=3				  *
			 * Frequency Stats Message: dataRate=-1 && SNR=1              *
			 * QoS Stats Message      : dataRate=-1                       *
			 * ---------------------------------------------------------- *
			 * For others Info: see  CWBinding.c                          *
			 **************************************************************/

			/* In this function is tied the name of the socket: recSock */
			CW_CREATE_OBJECT_ERR(bindingValuesPtr, CWBindingTransportHeaderValues, EXIT_THREAD);
			bindingValuesPtr->dataRate = -1;
			bindingValuesPtr->SNR = 3;

			/* Capwap Message Assembling */

			if (CWAssembleDataMessage(&completeMsgPtr, &fragmentsNum, gWTPPathMTU, data,
					bindingValuesPtr,
#ifdef CW_NO_DTLS
					CW_PACKET_PLAIN
#else
					CW_PACKET_CRYPT
#endif
			) == CW_TRUE)

			{
				for (k = 0; k < fragmentsNum; k++) {

#ifdef CW_NO_DTLS
					if (!CWNetworkSendUnsafeConnected(gWTPSocket, completeMsgPtr[k].msg, completeMsgPtr[k].offset))
#else
					if (!CWSecuritySend(gWTPSession, completeMsgPtr[k].msg,
							completeMsgPtr[k].offset))
#endif
					{
						CWDebugLog("Failure sending Request");
					}
				}
			}

			/* Free used Structures */

			for (k = 0; k < fragmentsNum; k++) {
				CW_FREE_PROTOCOL_MESSAGE(completeMsgPtr[k]);
			}

			CW_FREE_OBJECT(completeMsgPtr);
			CW_FREE_PROTOCOL_MESSAGE(*data);
			CW_FREE_OBJECT(data);
			CW_FREE_OBJECT(bindingValuesPtr);
			CW_FREE_OBJECT(buffer);

			munmap(shared_msg, shared_seg_size);

		} else {
			CWDebugLog(
					"[CLIENT APP_NetlinkServer]:Thread Station Info Receive Stats: Error on recvfrom");
			continue;
		}
	}

	CWDebugLog(
			"[CLIENT APP_NetlinkServer]:Thread Station Info Receive Stats: Thread ended unexpectedly!!");
	shm_remove_shared();
	CWExitThread();
}
