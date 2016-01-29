/*******************************************************************************************
 * Copyright (c) 2009 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
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


#ifndef __CAPWAP_WTPStaInfoReceive_HEADER__
#define __CAPWAP_WTPStaInfoReceive_HEADER__

#include "CWWTP.h"
#include <ctype.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>

#define SERVER_PORT 1238
#define PACKET_SIZE 65536
/* Posix IPC object name [system dependant] - see
 *
 */
#define WTP_STAINFO_SHARED_MEMORY_PATH         "/WTPStaInfo_SharedMemory"
/* maximum length of the content of the message */
#define MAX_MSG_LENGTH      65536

#define STAINFO_SLEEP_TIME 10

/* message structure for messages in the shared segment */
struct msg_s {
	int type;
	char content[MAX_MSG_LENGTH];
};


#define bzero(ptr, len)  memset((ptr),(int) NULL, (len))

#define EXIT_THREAD CWLog("ERROR Handling Station Info Stats: application will be closed!");		\
				CWExitThread();


#endif

