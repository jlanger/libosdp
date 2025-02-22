/*
 * Copyright (c) 2024 Siddharth Chandrasekaran <sidcha.dev@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <utils/pcap_gen.h>

#include "osdp_common.h"

void osdp_packet_capture_init(struct osdp_pd *pd)
{
	pcap_t *cap;
	int n;
	char path[128];

	n = snprintf(path, sizeof(path), "osdp-trace-%spd-%d-",
		     is_pd_mode(pd) ? "" : "cp-", pd->address);
	n += add_iso8601_utc_datetime(path + n, sizeof(path) - n);
	strcpy(path + n, ".pcap");
	cap = pcap_start(path, OSDP_PACKET_BUF_SIZE, OSDP_PCAP_LINK_TYPE);
	if (cap) {
		LOG_WRN("Capturing packets to '%s'", path);
		LOG_WRN("A graceful teardown of libosdp ctx is required"
			" for a complete trace file to be produced.");
	} else {
		LOG_ERR("Packet capture init failed; check if path '%s'"
			" is accessible", path);
	}
	pd->packet_capture_ctx = (void *)cap;
}

void osdp_packet_capture_finish(struct osdp_pd *pd)
{
	pcap_t *cap = pd->packet_capture_ctx;
	size_t num_packets;

	assert(cap);
	num_packets = cap->num_packets;
	if (pcap_stop(cap)) {
		LOG_ERR("Unable to stop capture (flush/close failed)");
		return;
	}
	LOG_INF("Captured %d packets", num_packets);
}

void osdp_capture_packet(struct osdp_pd *pd, uint8_t *buf, int len)
{
	pcap_t *cap = pd->packet_capture_ctx;

	assert(cap);
	assert(len <= OSDP_PACKET_BUF_SIZE);
	pcap_add(cap, buf, len);
}
