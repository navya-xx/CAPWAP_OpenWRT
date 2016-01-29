/*
 * Created by Navneet
 * Header file for netlinkTest.c
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/un.h>

#include "CWWTP.h"

#define SOCKET_PATH_RECV_AGENT "/tmp/monitorclt"
#define MAX_INPUT_SIZE 65536

struct bit_rate_info {
	uint16_t bitrate16;
	uint32_t bitrate32;
	uint8_t MCS_index;
	int Mhz_width40;
	int Mhz_width80;
	int Mhz_width160;
	int Mhz_width80P80;
	int rate_short_GI;
	uint8_t MCS_index_VHT;
	uint8_t num_streams_VHT;

};
struct sta_info {
	char* mac_addr;
	uint32_t inactive_time;
	uint32_t rx_bytes; //received bytes from this station
	uint32_t tx_bytes; // transmitted bytes to this station 32 bit implementation
	uint8_t signal_strgth; //in dB

	struct bit_rate_info txbitrate;
	struct bit_rate_info rxbitrate;

	uint32_t rx_packets;
	uint32_t tx_packets;
	uint32_t tx_retries;
	uint32_t tx_failed;
	uint8_t signal_avg;
	uint16_t mesh_LLID;
	uint16_t mesh_PLID;
	uint8_t plink_state;

	/* @NL80211_STA_INFO_BSS_PARAM: current station's view of BSS, nested attribute
	 containing info as possible, see &enum nl80211_sta_bss_param.
	 This functionality has not been implemented yet.

	 */

	/* Station flags are retrieve and put independently */

	int assoc;
	int auth;
	char* preamble;
	int WME_cbl;
	int MFP_cbl;
	int TDLS_peer;

	uint32_t beacon_loss;
	uint64_t timing_offset;
	uint32_t connected_time;

	uint32_t local_pm;
	uint32_t peer_pm;
	uint32_t nonpeer_pm;

	uint32_t throughput;

/* Not implementing the chain level signal strength in a PUDM*/
};
