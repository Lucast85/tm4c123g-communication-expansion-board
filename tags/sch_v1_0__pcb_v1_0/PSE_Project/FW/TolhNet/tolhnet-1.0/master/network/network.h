//=========================================================================
// network.h
// Example implementation of the network-layer code
/*=========================================================================
Copyright (C) 2013-2014 Giorgio Biagetti.

Distributed under the Boost Software License, Version 1.0.

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

=========================================================================*/

#ifndef __NETWORK_H
#define __NETWORK_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Define the NWK_DEBUG_DUMP macro to have a complete log of packet processing:

#ifndef __cplusplus
#undef NWK_DEBUG_DUMP
#endif

// Command code definitions
#define CODE_ACK     0
#define CODE_NACK    1
#define CODE_GET     2
#define CODE_TRACE   3
#define CODE_MSG     4
#define CODE_PING    5
#define CODE_SET     6
#define CODE_CONFIG  7


// Data structure to interface with the data-link layer:
struct packet
{
	char     device;
	uint8_t  flags;
	int8_t   signal_strength;
	int8_t   signal_quality;
	uint32_t timestamp;
	size_t   length;
	void    *data;
#ifdef NWK_DEBUG_DUMP
	void dump (const char *prefix, uint64_t node_address) const;
	static const char opcode_names[2][8];
#endif
};

// Call interface into the data-link layer:
typedef int (*send_packet_t) (struct packet const *p);

// Data structure to hold a FIB entry:
#if defined(__cplusplus) || defined(__NETWORK_C)
struct fib_entry
{
	uint16_t net;
	uint16_t mask;
	uint8_t  mask_length;
	uint8_t  type;
	int8_t   direction;
	char     device;
};
#endif

// FIB handling:
#ifdef __cplusplus
class fib
{
public:
#endif
	void fib_flush ();
	bool fib_add (uint16_t net, uint8_t length, char device, int direction);
	bool fib_del (uint16_t net, uint8_t length);
	bool fib_add_span (uint16_t first, uint16_t last, char device, int direction);
	bool fib_del_span (uint16_t first, uint16_t last);
	struct fib_entry const *fib_lookup (uint16_t address);
#ifdef NWK_DEBUG_DUMP
	void fib_dump () const;
#endif
#ifdef __cplusplus
protected:
#endif
#if defined(__cplusplus) || defined(__NETWORK_C)
	enum {fib_max_entries = 64};
	unsigned fib_size;
	struct fib_entry fib_table[fib_max_entries];
#endif
#ifdef __cplusplus
};
#endif

// Network handling:
#ifdef __cplusplus
class router : public fib
{
public:
	router (uint64_t mac);
	uint64_t address () const;
#endif
	void nwk_init (uint64_t mac);
	// lower-layer interface:
	int  nwk_connect_device (char device, send_packet_t send_packet);
	void packet_received (struct packet const *p);
	// upper-layer interface:
	enum nwk_events {
		nwk_network_down,   // network has been shut down.
		nwk_network_up,     // network has been configured.
		nwk_network_reconf  // network config has changed.
	};
	void (*netevent_received) (enum nwk_events event);
	void (*datagram_received) (uint16_t src_address, uint16_t sequence, int8_t code, void *data, size_t length);
	void (*netreply_received) (uint16_t src_address, uint16_t sequence, int8_t code, void *data, size_t length);
	int send_datagram         (uint16_t dst_address, const void *data, size_t length, uint8_t code);
	int send_nwk              (uint16_t dst_address, const void *data, size_t length, uint8_t code);
	int send_ping             (uint16_t dst_address, const void *data, size_t length);
	int send_trace            (uint16_t dst_address, uint16_t start_hop, uint16_t max_hops);
	// remote network configuration interface:
	int nwk_config_prepare    (uint16_t dst_address, uint64_t mac_address, uint16_t distance);
	int nwk_config_add_mask   (uint16_t network, uint8_t length, char device);
	int nwk_config_add_span   (uint16_t first, uint16_t last, char device);
	int nwk_packet_size       (void);
	int nwk_send_buffer       (void);
	// configuration data:
#if !defined(__cplusplus) && !defined(__NETWORK_C)
	extern
#endif
	struct {
		uint8_t enable_incoming : 1;
		uint8_t enable_forward  : 1;
		uint8_t enable_ping     : 1;
		uint8_t enable_config   : 1;
	} options;
#ifdef __cplusplus
private:
#endif
	void nwk_hash_clear (void);
	int nwk_hash_address (uint16_t address);
	int create_packet (bool target, uint16_t dst, uint16_t hop, uint64_t mac, uint8_t code, const void *data, size_t length);
	int send_packet (struct packet const *p);
#if defined(__cplusplus) || defined(__NETWORK_C)
	enum {
		max_packet_length   = 250,
		max_interfaces      = 8,   // e.g.: A, B, C, D, U, P, W, N
#ifdef __cplusplus
		dst_hash_table_size = 65536
#else
		dst_hash_table_size = 16
#endif
	};
	uint64_t nwk_station_address;
	uint16_t nwk_depth_level;
	uint16_t nwk_tx_sequence_counter;
	uint16_t nwk_rx_sequence_counter;
	uint16_t nwk_rx_sequence_source;
	struct net_interface {
		char device;
		send_packet_t send_packet;
	} connections[max_interfaces];
	uint16_t dst_hash_table[dst_hash_table_size];
	uint16_t seq_hash_table[dst_hash_table_size];
	uint8_t packet_buffer[max_packet_length];
	struct packet outgoing_packet;
#endif
#ifdef __cplusplus
};
#endif

uint8_t log_encode (uint32_t x);
uint32_t log_decode (uint8_t n);

#endif // __NETWORK_H
