/*******************************************************************************************
 * Copyright (c) 2008 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
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
 * Author : Daniele De Sanctis (danieledesanctis@gmail.com)                                *
 *                                            						   *
 *******************************************************************************************/

#ifndef __CAPWAP_WTPStatsReceive_HEADER__
#define __CAPWAP_WTPStatsReceive_HEADER__

#include "CWWTP.h"
#include <ctype.h>
#include <netinet/in.h>
#include <sys/un.h>


#define SOCKET_PATH     "/tmp/wtp"
#define PACKET_SIZE						65536


#define bzero(ptr, len)  memset((ptr),(int) NULL, (len))	



#define EXIT_THREAD		CWLog("ERROR Handling Stats: application will be closed!");		\
				close(sock);								\
				exit(1);




int create_data_Frame(CWProtocolMessage** frame,  char* buffer, int len);
#endif
