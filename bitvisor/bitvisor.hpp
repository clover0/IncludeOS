void bv_nop(void);

void bv_yield(void);

int bv_net_write(char *buf, int size);

int bv_net_read(char *buf, int size);

int bv_block_write(char *buf, int offset, int size);

int bv_block_read(char *buf, int offset, int size);

int bv_get_time(unsigned long *time);
