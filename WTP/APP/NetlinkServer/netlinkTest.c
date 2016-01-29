/*
 * Created by Navneet
 */

#include "netlinkTest.h"

void mac_addr_n2a(char *mac_addr, unsigned char *arg) {
	int i, l;

	l = 0;
	for (i = 0; i < ETH_ALEN; i++) {
		if (i == 0) {
			sprintf(mac_addr + l, "%02x", arg[i]);
			l += 2;
		} else {
			sprintf(mac_addr + l, ":%02x", arg[i]);
			l += 3;
		}
	}
}
int mac_addr_a2n(unsigned char *mac_addr, char *arg) {
	int i;

	for (i = 0; i < ETH_ALEN; i++) {
		int temp;
		char *cp = strchr(arg, ':');
		if (cp) {
			*cp = 0;
			cp++;
		}
		if (sscanf(arg, "%x", &temp) != 1)
			return -1;
		if (temp < 0 || temp > 255)
			return -1;

		mac_addr[i] = temp;
		if (!cp)
			break;
		arg = cp;
	}
	if (i < ETH_ALEN - 1)
		return -1;

	return 0;
}

static int phy_lookup(char *name) {
	char buf[200];
	int fd, pos;

	snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/index", name);

	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return -1;
	pos = read(fd, buf, sizeof(buf) - 1);
	if (pos < 0) {
		close(fd);
		return -1;
	}
	buf[pos] = '\0';
	close(fd);
	return atoi(buf);
}

static void print_power_mode(struct nlattr *a) {
	enum nl80211_mesh_power_mode pm = nla_get_u32(a);

	switch (pm) {
	case NL80211_MESH_POWER_ACTIVE:
		printf("ACTIVE");
		break;
	case NL80211_MESH_POWER_LIGHT_SLEEP:
		printf("LIGHT SLEEP");
		break;
	case NL80211_MESH_POWER_DEEP_SLEEP:
		printf("DEEP SLEEP");
		break;
	default:
		printf("UNKNOWN");
		break;
	}
}

void parse_bitrate(struct nlattr *bitrate_attr, char *buf, int buflen,
		struct bit_rate_info* bit_rate_data) {
	int rate = 0;
	char *pos = buf;
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
	bit_rate_data->Mhz_width160 = 0;
	bit_rate_data->Mhz_width40 = 0;
	bit_rate_data->Mhz_width80 = 0;
	bit_rate_data->Mhz_width80P80 = 0;
	bit_rate_data->rate_short_GI = 0;
	//below is a way of initializing an array of structs nla_policy
	static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = { [NL80211_RATE_INFO_BITRATE
			] = { .type = NLA_U16 }, [NL80211_RATE_INFO_BITRATE32] = { .type = NLA_U32 },
			[NL80211_RATE_INFO_MCS] = { .type = NLA_U8 }, [NL80211_RATE_INFO_40_MHZ_WIDTH] = {
					.type = NLA_FLAG }, [NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG }, };

	if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX, bitrate_attr, rate_policy)) {
		snprintf(buf, buflen, "failed to parse nested rate attributes!");
		return;
	}

	if (rinfo[NL80211_RATE_INFO_BITRATE32]) {
		rate = nla_get_u32(rinfo[NL80211_RATE_INFO_BITRATE32]);
		bit_rate_data->bitrate32 = rate;
	} else if (rinfo[NL80211_RATE_INFO_BITRATE]) {
		rate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
		bit_rate_data->bitrate16 = rate;
	}
	if (rate > 0)
		pos += snprintf(pos, buflen - (pos - buf), "%d.%d MBit/s", rate / 10, rate % 10);

	if (rinfo[NL80211_RATE_INFO_MCS])
		pos += snprintf(pos, buflen - (pos - buf), " MCS %d",
				bit_rate_data->MCS_index = nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]));
	if (rinfo[NL80211_RATE_INFO_VHT_MCS])
		pos += snprintf(pos, buflen - (pos - buf), " VHT-MCS %d",
				bit_rate_data->MCS_index_VHT = nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_MCS]));
	if (rinfo[NL80211_RATE_INFO_40_MHZ_WIDTH]) {
		pos += snprintf(pos, buflen - (pos - buf), " 40MHz");
		bit_rate_data->Mhz_width40 = 1;
	}
	if (rinfo[NL80211_RATE_INFO_80_MHZ_WIDTH]) {
		pos += snprintf(pos, buflen - (pos - buf), " 80MHz");
		bit_rate_data->Mhz_width80 = 1;
	}
	if (rinfo[NL80211_RATE_INFO_80P80_MHZ_WIDTH]) {
		pos += snprintf(pos, buflen - (pos - buf), " 80P80MHz");
		bit_rate_data->Mhz_width80P80 = 1;
	}
	if (rinfo[NL80211_RATE_INFO_160_MHZ_WIDTH]) {
		pos += snprintf(pos, buflen - (pos - buf), " 160MHz");
		bit_rate_data->Mhz_width160 = 1;
	}
	if (rinfo[NL80211_RATE_INFO_SHORT_GI]) {
		pos += snprintf(pos, buflen - (pos - buf), " short GI");
		bit_rate_data->rate_short_GI = 1;
	}
	if (rinfo[NL80211_RATE_INFO_VHT_NSS])
		pos += snprintf(pos, buflen - (pos - buf), " VHT-NSS %d",
				bit_rate_data->num_streams_VHT = nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_NSS]));
}

static char *get_chain_signal(struct nlattr *attr_list) {
	struct nlattr *attr;
	static char buf[64];
	char *cur = buf;
	int i = 0, rem;
	const char *prefix;

	if (!attr_list)
		return "";

	nla_for_each_nested(attr, attr_list, rem) {
		if (i++ > 0)
		prefix = ", ";
		else
		prefix = "[";

		cur += snprintf(cur, sizeof(buf) - (cur - buf), "%s%d", prefix,
				(int8_t) nla_get_u8(attr));
	}

	if (i)
		snprintf(cur, sizeof(buf) - (cur - buf), "] ");

	return buf;
}

int nl80211_init(struct nl80211_state *state) {
	int err;

	state->nl_sock = nl_socket_alloc();
	if (!state->nl_sock) {
		fprintf(stderr, "Failed to allocate netlink socket.\n");
		return -ENOMEM;
	}

	nl_socket_set_buffer_size(state->nl_sock, 8192, 8192);

	if (genl_connect(state->nl_sock)) {
		fprintf(stderr, "Failed to connect to generic netlink.\n");
		err = -ENOLINK;
		goto out_handle_destroy;
	}

	state->nl80211_id = genl_ctrl_resolve(state->nl_sock, "nl80211");
	if (state->nl80211_id < 0) {
		fprintf(stderr, "nl80211 not found.\n");
		err = -ENOENT;
		goto out_handle_destroy;
	}

	return 0;

	out_handle_destroy: nl_socket_free(state->nl_sock);
	return err;
}

static void nl80211_cleanup(struct nl80211_state *state) {
	nl_socket_free(state->nl_sock);
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg) {
	int *ret = arg;
	*ret = err->error;
	return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg) {
	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg) {
	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static int station_dump_handler(struct nl_msg *msg, struct sta_info *arg) {
	/*
	 * Dynamic memory allocation code did not work.
	 */
	//	int sta_total = 1;
	//	struct sta_info* temp = arg;
	//	while (!temp->mac_addr==NULL)
	//	{
	//		temp++;
	//		sta_total++;
	//	} //computed which position station to read
	//	//allocated new memory for the incoming station
	//	int new_size = (1+sta_total)*sizeof(struct sta_info);
	//	arg = (struct sta_info*)realloc(arg,new_size);
	//	while (sta_total!=1)
	//	{
	//		arg++;
	//		sta_total--;
	//	}
	//	(arg+1)->mac_addr=NULL;

	while (!arg->mac_addr == NULL) {
		arg++;
	}
	(arg + 1)->mac_addr = NULL;
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg)); // pointer to the payload
	struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
	char mac_addr[20], state_name[10], dev[20];
	struct nl80211_sta_flag_update *sta_flags;
	//below is a way of initializing an array of structs nla_policy
	static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
			[NL80211_STA_INFO_INACTIVE_TIME] = { .type = NLA_U32 }, [NL80211_STA_INFO_RX_BYTES] = {
					.type = NLA_U32 }, [NL80211_STA_INFO_TX_BYTES] = { .type = NLA_U32 },
			[NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 }, [NL80211_STA_INFO_TX_PACKETS] = {
					.type = NLA_U32 }, [NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
			[NL80211_STA_INFO_T_OFFSET] = { .type = NLA_U64 }, [NL80211_STA_INFO_TX_BITRATE] = {
					.type = NLA_NESTED }, [NL80211_STA_INFO_RX_BITRATE] = { .type = NLA_NESTED },
			[NL80211_STA_INFO_LLID] = { .type = NLA_U16 }, [NL80211_STA_INFO_PLID] = { .type =
					NLA_U16 }, [NL80211_STA_INFO_PLINK_STATE] = { .type = NLA_U8 },
			[NL80211_STA_INFO_TX_RETRIES] = { .type = NLA_U32 }, [NL80211_STA_INFO_TX_FAILED] = {
					.type = NLA_U32 }, [NL80211_STA_INFO_STA_FLAGS] =
			{ .minlen = sizeof(struct nl80211_sta_flag_update) }, [NL80211_STA_INFO_LOCAL_PM] = {
					.type = NLA_U32 }, [NL80211_STA_INFO_PEER_PM] = { .type = NLA_U32 },
			[NL80211_STA_INFO_NONPEER_PM] = { .type = NLA_U32 }, [NL80211_STA_INFO_CHAIN_SIGNAL
					] = { .type = NLA_NESTED }, [NL80211_STA_INFO_CHAIN_SIGNAL_AVG] = { .type =
					NLA_NESTED }, };
	char *chain;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	/*
	 * TODO: validate the interface and mac address!
	 * Otherwise, there's a race condition as soon as
	 * the kernel starts sending station notifications.
	 */

	if (!tb[NL80211_ATTR_STA_INFO]) {
		fprintf(stderr, "sta stats missing!\n");
		return NL_SKIP;
	}
	if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX, tb[NL80211_ATTR_STA_INFO], stats_policy)) {
		fprintf(stderr, "failed to parse nested attributes!\n");
		return NL_SKIP;
	}

	mac_addr_n2a(mac_addr, nla_data(tb[NL80211_ATTR_MAC]));
	arg->mac_addr = mac_addr;
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);
	printf("Station %s (on %s)", mac_addr, dev);

	if (sinfo[NL80211_STA_INFO_INACTIVE_TIME])
		printf("\n\tinactive time:\t%u ms",
				arg->inactive_time = nla_get_u32(sinfo[NL80211_STA_INFO_INACTIVE_TIME]));
	if (sinfo[NL80211_STA_INFO_RX_BYTES])
		printf("\n\trx bytes:\t%u",
				arg->rx_bytes = nla_get_u32(sinfo[NL80211_STA_INFO_RX_BYTES]));
	if (sinfo[NL80211_STA_INFO_RX_PACKETS])
		printf("\n\trx packets:\t%u",
				arg->rx_packets = nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]));
	if (sinfo[NL80211_STA_INFO_TX_BYTES])
		printf("\n\ttx bytes:\t%u",
				arg->tx_bytes = nla_get_u32(sinfo[NL80211_STA_INFO_TX_BYTES]));
	if (sinfo[NL80211_STA_INFO_TX_PACKETS])
		printf("\n\ttx packets:\t%u",
				arg->tx_packets = nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]));
	if (sinfo[NL80211_STA_INFO_TX_RETRIES])
		printf("\n\ttx retries:\t%u",
				arg->tx_retries = nla_get_u32(sinfo[NL80211_STA_INFO_TX_RETRIES]));
	if (sinfo[NL80211_STA_INFO_TX_FAILED])
		printf("\n\ttx failed:\t%u",
				arg->tx_failed = nla_get_u32(sinfo[NL80211_STA_INFO_TX_FAILED]));

	chain = get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL]);
	if (sinfo[NL80211_STA_INFO_SIGNAL])
		printf("\n\tsignal:  \t%d %sdBm",
				arg->signal_strgth = (int8_t) nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]), chain);

	chain = get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL_AVG]);
	if (sinfo[NL80211_STA_INFO_SIGNAL_AVG])
		printf("\n\tsignal avg:\t%d %sdBm",
				arg->signal_avg = (int8_t) nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL_AVG]), chain);

	if (sinfo[NL80211_STA_INFO_T_OFFSET])
		printf(
				"\n\tToffset:\t%lld us",
				arg->timing_offset = (unsigned long long) nla_get_u64(
						sinfo[NL80211_STA_INFO_T_OFFSET]));

	if (sinfo[NL80211_STA_INFO_TX_BITRATE]) {
		char buf[100];
		struct bit_rate_info txbitrate_info;
		parse_bitrate(sinfo[NL80211_STA_INFO_TX_BITRATE], buf, sizeof(buf), &txbitrate_info);
		arg->txbitrate = txbitrate_info;
		printf("\n\ttx bitrate:\t%s", buf);
	}

	if (sinfo[NL80211_STA_INFO_RX_BITRATE]) {
		char buf[100];
		struct bit_rate_info rxbitrate_info;
		parse_bitrate(sinfo[NL80211_STA_INFO_RX_BITRATE], buf, sizeof(buf), &rxbitrate_info);
		arg->rxbitrate = rxbitrate_info;
		printf("\n\trx bitrate:\t%s", buf);
	}

	if (sinfo[NL80211_STA_INFO_EXPECTED_THROUGHPUT]) {
		uint32_t thr;

		thr = nla_get_u32(sinfo[NL80211_STA_INFO_EXPECTED_THROUGHPUT]);
		/* convert in Mbps but scale by 1000 to save kbps units */
		thr = thr * 1000 / 1024;
		arg->throughput = thr;
		printf("\n\texpected throughput:\t%u.%uMbps", thr / 1000, thr % 1000);
	}

	if (sinfo[NL80211_STA_INFO_LLID])
		printf("\n\tmesh llid:\t%d", nla_get_u16(sinfo[NL80211_STA_INFO_LLID]));
	if (sinfo[NL80211_STA_INFO_PLID])
		printf("\n\tmesh plid:\t%d", nla_get_u16(sinfo[NL80211_STA_INFO_PLID]));
	if (sinfo[NL80211_STA_INFO_PLINK_STATE]) {
		switch (nla_get_u8(sinfo[NL80211_STA_INFO_PLINK_STATE])) {
		case LISTEN:
			strcpy(state_name, "LISTEN");
			break;
		case OPN_SNT:
			strcpy(state_name, "OPN_SNT");
			break;
		case OPN_RCVD:
			strcpy(state_name, "OPN_RCVD");
			break;
		case CNF_RCVD:
			strcpy(state_name, "CNF_RCVD");
			break;
		case ESTAB:
			strcpy(state_name, "ESTAB");
			break;
		case HOLDING:
			strcpy(state_name, "HOLDING");
			break;
		case BLOCKED:
			strcpy(state_name, "BLOCKED");
			break;
		default:
			strcpy(state_name, "UNKNOWN");
			break;
		}
		printf("\n\tmesh plink:\t%s", state_name);
	}
	if (sinfo[NL80211_STA_INFO_LOCAL_PM]) {
		printf("\n\tmesh local PS mode:\t");
		print_power_mode(sinfo[NL80211_STA_INFO_LOCAL_PM]);
	}
	if (sinfo[NL80211_STA_INFO_PEER_PM]) {
		printf("\n\tmesh peer PS mode:\t");
		print_power_mode(sinfo[NL80211_STA_INFO_PEER_PM]);
	}
	if (sinfo[NL80211_STA_INFO_NONPEER_PM]) {
		printf("\n\tmesh non-peer PS mode:\t");
		print_power_mode(sinfo[NL80211_STA_INFO_NONPEER_PM]);
	}

	if (sinfo[NL80211_STA_INFO_STA_FLAGS]) {
		sta_flags = (struct nl80211_sta_flag_update *) nla_data(sinfo[NL80211_STA_INFO_STA_FLAGS]);

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
			printf("\n\tauthorized:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
				printf("yes");
				arg->auth = 1;
			} else {
				printf("no");
				arg->auth = 0;
			}
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_AUTHENTICATED)) {
			printf("\n\tauthenticated:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_AUTHENTICATED)) {
				printf("yes");
				arg->assoc = 1;
			} else {
				printf("no");
				arg->assoc = 0;
			}
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE)) {
			printf("\n\tpreamble:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE)) {
				printf("short");
				arg->preamble = "short";
			} else {
				printf("long");
				arg->preamble = "long";
			}
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_WME)) {
			printf("\n\tWMM/WME:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_WME)) {
				printf("yes");
				arg->WME_cbl = 1;
			} else {
				printf("no");
				arg->WME_cbl = 0;
			}
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_MFP)) {
			printf("\n\tMFP:\t\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_MFP)) {
				printf("yes");
				arg->MFP_cbl = 1;
			} else {
				printf("no");
				arg->MFP_cbl = 0;
			}
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
			printf("\n\tTDLS peer:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
				printf("yes");
				arg->TDLS_peer = 1;
			} else {
				printf("no");
				arg->TDLS_peer = 0;
			}
		}
	}

	printf("\n");
	arg++;
	return NL_SKIP;
}

int station_dump(struct nl_msg* msg, struct nl80211_state* state, int devidx,
		struct sta_info* out_test, int* num_station) {
	/*
	 * TODO: Cases of multiple station connected
	 * */

	struct nl_cb *cb;
	struct nl_cb *s_cb;
	cb = nl_cb_alloc(NL_CB_DEFAULT);
	s_cb = nl_cb_alloc(NL_CB_DEFAULT);
	int err;
	if (!cb || !s_cb) {
		fprintf(stderr, "failed to allocate netlink callbacks\n");
		err = 2;
		goto out_free_msg;
	}

	genlmsg_put(msg, 0, 0, state->nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_STATION, 0);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, station_dump_handler, out_test);
	nl_socket_set_cb(state->nl_sock, s_cb);

	err = nl_send_auto_complete(state->nl_sock, msg);
	if (err < 0)
		goto out;

	err = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	while (err > 0)
		nl_recvmsgs(state->nl_sock, cb);

	*num_station = 0;
	struct sta_info* temp_pointer = out_test;
	while (!temp_pointer->mac_addr == NULL) {
		*num_station = *num_station + 1;
		temp_pointer++;
	}
	out: nl_cb_put(cb);
	out_free_msg: nlmsg_free(msg);
	goto cleanup;
	nla_put_failure: fprintf(stderr, "building message failed\n");
	goto cleanup;

	cleanup: nl80211_cleanup(state);
	return 0;
}
int station_delete(char* mac, struct nl_msg* msg, struct nl80211_state* state, int devidx) {
	printf("%s \n", mac);
	struct nl_cb *cb;
	struct nl_cb *s_cb;
	cb = nl_cb_alloc(NL_CB_DEFAULT);
	s_cb = nl_cb_alloc(NL_CB_DEFAULT);
	int err;
	if (!cb || !s_cb) {
		fprintf(stderr, "failed to allocate netlink callbacks\n");
		err = 2;
		goto out_free_msg;
	}

	genlmsg_put(msg, 0, 0, state->nl80211_id, 0, 0, NL80211_CMD_DEL_STATION, 0);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);
	unsigned char mac_addr[ETH_ALEN];

	if (mac_addr_a2n(mac_addr, mac)) {
		printf(stderr, "invalid mac address\n");
		return 2;
	}

	NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, mac_addr);

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, station_dump_handler, NULL);
	nl_socket_set_cb(state->nl_sock, s_cb);

	err = nl_send_auto_complete(state->nl_sock, msg);
	if (err < 0)
		goto out;

	err = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	while (err > 0)
		nl_recvmsgs(state->nl_sock, cb);

	out: nl_cb_put(cb);
	out_free_msg: nlmsg_free(msg);
	goto cleanup;
	nla_put_failure: fprintf(stderr, "building message failed\n");
	goto cleanup;

	cleanup: nl80211_cleanup(state);
	return 0;
}

int station_get(char* mac, struct nl_msg* msg, struct nl80211_state* state, int devidx,
		struct sta_info* station_info_get) {
	printf("%s \n", mac);
	struct nl_cb *cb;
	struct nl_cb *s_cb;
	cb = nl_cb_alloc(NL_CB_DEFAULT);
	s_cb = nl_cb_alloc(NL_CB_DEFAULT);
	int err;
	if (!cb || !s_cb) {
		fprintf(stderr, "failed to allocate netlink callbacks\n");
		err = 2;
		goto out_free_msg;
	}

	genlmsg_put(msg, 0, 0, state->nl80211_id, 0, 0, NL80211_CMD_GET_STATION, 0);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);
	unsigned char mac_addr[ETH_ALEN];

	if (mac_addr_a2n(mac_addr, mac)) {
		printf(stderr, "invalid mac address\n");
		return 2;
	}

	NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, mac_addr);

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, station_dump_handler, station_info_get);
	nl_socket_set_cb(state->nl_sock, s_cb);

	err = nl_send_auto_complete(state->nl_sock, msg);
	if (err < 0)
		goto out;

	err = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	while (err > 0)
		nl_recvmsgs(state->nl_sock, cb);

	out: nl_cb_put(cb);
	out_free_msg: nlmsg_free(msg);
	goto cleanup;
	nla_put_failure: fprintf(stderr, "building message failed\n");
	goto cleanup;

	cleanup: nl80211_cleanup(state);
	return 0;
}
