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
 * Author : Antonio Davoli (antonio.davoli@gmail.com)                                      *
 *                                            				                    		   *
 *******************************************************************************************/

#include "WTPFreqManager.h"
CWBool CWSetCurrentChannel(int channel);

CW_THREAD_RETURN_TYPE CWWTPFreqManager(void *arg) {
	int recSock, rlen;

	struct sockaddr_in servaddr, client_addr;
	socklen_t slen = sizeof(client_addr);

	char buffer[PACKET_SIZE];
	OFDMControlValues* freqValue;
	int current_chan=0;

	CWProtocolMessage *completeMsgPtr = NULL;

	CWThreadSetSignals(SIG_BLOCK, 1, SIGALRM);

	/* Create an Inet UDP socket for this thread (Receive freq/ack packets) */

	if ((recSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		CWDebugLog("Thread Frequency Management: Error creating socket");
		CWExitThread();
	}

	/*  Set up address structure for server socket */

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	servaddr.sin_port = htons(SERVER_PORT);

	/* Binding Socket */

	if (bind(recSock, (struct sockaddr *) &servaddr, sizeof(struct sockaddr_in)) < 0) {
		CWDebugLog("Thread Frequency Management: Binding Socket Error");
		close(recSock);
		CWExitThread();
	}

	CW_REPEAT_FOREVER /* Receive data Loop */
	{
		memset(buffer, 0, PACKET_SIZE);
		rlen = 0;

		if ( ( rlen = recvfrom(recSock, buffer, PACKET_SIZE, 0, (struct sockaddr *) &client_addr, &slen) ) > 0 )
		{
			freqValue = (OFDMControlValues*) buffer;
			current_chan = freqValue->currentChan;
			if(!CWSetCurrentChannel(current_chan)){
				CWLog("Thread Frequency Management: Failed to set current channel value.");
				continue;
			}

		}
	}

		CWDebugLog("Thread Frequency Management: Thread ended unexpectedly!!");
		close(recSock);
		CWExitThread();
}

CWBool CWSetCurrentChannel(int channel) {
	char* chan;
	sprintf(chan, "%d", channel);
	// uci command
	char cmd[100];
	strcpy(cmd, "uci set wireless.@wifi-device[0].channel=");
	strcat(cmd, chan);
	strcat(cmd, "; uci commit wireless; wifi");
	SYSTEM_ERR(cmd);
	return CW_TRUE;
}
