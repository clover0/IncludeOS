#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void bv_nop(void);

void bv_yield(void);

int bv_net_write(char *buf, int size);

int bv_net_read(char *buf, int size);

int bv_block_write(char *buf, int offset, int size);

int bv_block_read(char *buf, int offset, int size);

int bv_get_time(unsigned long *time);

int bv_msgopen(const char *name);

int bv_msgsendint(int desc, int data);

void bv_console_write(const char *buf, unsigned long size);

struct bv_start_info {
    const char *cmdline;
    uintptr_t heap_start;
    size_t heap_size;
};