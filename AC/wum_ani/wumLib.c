#include <stdio.h>     
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>     
#include <string.h>    
#include <unistd.h>     
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "wumLib.h"

#define SA const struct sockaddr
#define BUF_SIZE 1024
#define QUIT_MSG 0




void set_channel(msg_elements *msg_payload, int value){
	char c = value;
	WUMPayloadStore16(msg_payload,OFDM_CONTROL_TYPE); // type
	WUMPayloadStore16(msg_payload,OFDM_CONTROL_LEN); //LEN
	WUMPayloadStore8(msg_payload, 0);// for Radio ID
	WUMPayloadStore8(msg_payload, 0);// for reserved space
	WUMPayloadStore8(msg_payload, c);// for channel value
	WUMPayloadStore8(msg_payload, 0);// for band support
	WUMPayloadStore32(msg_payload, 0);// for TI threshold

}

void set_txpower(msg_elements *msg_payload, int value){
	short int x = value;
	WUMPayloadStore16(msg_payload,TXPOWER_CONTROL_TYPE); // type
	WUMPayloadStore16(msg_payload,TXPOWER_CONTROL_LEN); //LEN
	WUMPayloadStore8(msg_payload, 0);// for Radio ID
	WUMPayloadStore8(msg_payload, 0);// for reserved space
	WUMPayloadStore16(msg_payload,x);// for transmit power
}

// function to build message according to capwap RFC
void build_msg(msg_elements *msg, msg_elements *msg_payload, int param_cnt, int wtp_id){
	WUMPayloadStore8(msg, 2);
	WUMPayloadStore32(msg, wtp_id);
	WUMPayloadStore32(msg, param_cnt);
	WUMPayloadStore32(msg, msg_payload->offset);
	WUMPayloadStoreRawBytes(msg, msg_payload);
}
int ACServerConnect(char *address, int port) {
		int sockfd, ret;
		struct sockaddr_in servaddr;

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket error:");
			exit(1);
		}

		bzero(&servaddr, sizeof(struct sockaddr_in));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(port);
		inet_pton(AF_INET, address, &servaddr.sin_addr);

		if (connect(sockfd, (SA*) &servaddr, sizeof(servaddr)) < 0) {
			perror("connect error:");
			exit(1);
		}

		if (Read32(sockfd, &ret) != 4) {
			exit(1);
		}

		if (ret == -1) {
			fprintf(stderr,
					"The AC Server's Client Queue Is Currently Full.\n");
			exit(1);
		} else if (ret != 1) {
			fprintf(stderr,
					"Something Wrong Happened While Connecting To The AC Server.\n");
			exit(1);
		}

		return sockfd;
	}

	void ACServerDisconnect(int acserver) {
		char msg = QUIT_MSG;
		if (Writen(acserver, &msg, 1) != 1) {
			fprintf(stderr, "Error while sending QUIT message.\n");
		}

		if (close(acserver) < 0) {
			perror("close error:");
		}
	}



	int /* Read "n" bytes from a descriptor. */
	readn(int fd, void *vptr, size_t n) {
		size_t nleft;
		ssize_t nread;
		char *ptr;

		ptr = vptr;
		nleft = n;
		while (nleft > 0) {
			if ((nread = recv(fd, ptr, nleft, 0)) < 0) {
				if (errno == EINTR)
					nread = 0; /* and call read() again */
				else
					return (-1);
			} else if (nread == 0)
				break; /* EOF */

			nleft -= nread;
			ptr += nread;
		}
		return (n - nleft); /* return >= 0 */
	}


	int Readn(int fd, void *ptr, size_t nbytes) {
			int n;

			if ((n = readn(fd, ptr, nbytes)) < 0)
				perror("readn error");
			return (n);
		}

		int /* Write "n" bytes to a descriptor. */
		writen(int fd, const void *vptr, size_t n) {
			size_t nleft;
			ssize_t nwritten;
			const char *ptr;

			ptr = vptr;
			nleft = n;
			while (nleft > 0) {
				if ((nwritten = send(fd, ptr, nleft, 0)) <= 0) {
					if (errno == EINTR)
						nwritten = 0; /* and call write() again */
					else
						return (-1); /* error */
				}

				nleft -= nwritten;
				ptr += nwritten;
			}
			return (n);
		}

		int Writen(int fd, void *ptr, size_t nbytes) {
			int n;
			while ((n = writen(fd, ptr, nbytes)) < 0)
				perror("writen error");
			return n;
		}

		int Read32(int fd, int *ptr) {
			int ret;

			ret = Readn(fd, ptr, 4);

			*ptr = ntohl(*ptr);

			return ret;
		}

		int Write32(int fd, void *ptr) {
		}

	void WUMPayloadStore8(msg_elements *msg_payload, char c) {
		msg_payload->payload[msg_payload->offset++] = c;
	}

	void WUMPayloadStore32(msg_elements *msg_payload, int i) {
		i = htonl(i);
		memcpy(msg_payload->payload + msg_payload->offset, &i, 4);
		msg_payload->offset += 4;
	}

	void WUMPayloadStore16(msg_elements *msg_payload, short int i) {
			i = htons(i);
			memcpy(msg_payload->payload + msg_payload->offset, &i, 2);
			msg_payload->offset += 2;
		}

	void WUMPayloadStoreRawBytes(msg_elements *msg, msg_elements *msg_payload) {
			memcpy(msg->payload + msg->offset, msg_payload->payload, msg_payload->offset);
			msg->offset += msg_payload->offset;
		}

	int WUMSendMessage(int acserver, msg_elements *msg) {
			/* Fix byte order issues */
			if (msg->offset> 0) {
				if (Writen(acserver, msg->payload, msg->offset)
						!= msg->offset) {
					fprintf(stderr,
							"Error while sending update message.\n");
					return ERROR;
				}
			}

			return SUCCESS;
		}

