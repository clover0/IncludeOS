#ifndef _BITVISOR_H
#define _BITVISOR_H

#include <stddef.h>
#include <stdint.h>

void bv_nop(void);

void bv_yield(void);

int bv_net_write(unsigned char *buf, size_t size);

int bv_net_read(unsigned char *buf, size_t size, size_t *rsize);

int bv_block_write(char *buf, int offset, int size);

int bv_block_read(char *buf, int offset, int size, int *rsize);

int bv_get_time(unsigned long *time);

int bv_msgopen(const char *name);

int bv_msgsendint(int desc, int data);

void bv_console_write(const char *buf, unsigned long size);

int bv_set_tls_base(uintptr_t base);

struct bv_start_info {
  const char *cmdline;
  uintptr_t heap_start;
  size_t heap_size;
};

/*
 * Network I/O.
 */

/*
 * Ethernet address length in bytes.
 */

#define BV_NET_ALEN 6
/*
 * Ethernet frame header (target, source, type) length in bytes.
 */

#define BV_NET_HLEN 14
struct bv_net_info {
  uint8_t mac_address[BV_NET_ALEN];
  size_t mtu; /* Not including Ethernet header */
};
#endif