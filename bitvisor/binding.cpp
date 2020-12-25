#include "bitvisor.hpp"

typedef unsigned long ulong;

#define CALLADDR 0x3FFFF000
#define SYS_NOP 1
#define SYS_MSGSENDINT 7
#define SYS_MSGOPEN 5

#define BV_YIELD 16
#define BV_NET_WRITE 17
#define BV_NET_READ 18
#define BV_BLOCK_WRITE 19
#define BV_BLOCK_READ 20
#define BV_GET_TIME 21

// We can use only x86_64 arch
#define DOSYSCALL0(rb, ra) asm volatile("call *%1"                             \
										: "=a"(ra)                             \
										: "0"((ulong)CALLADDR), "b"((ulong)rb) \
										: "memory", "cc", "%rcx", "%rdx", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15")
#define DOSYSCALL1(rb, rs, ra) asm volatile("call *%1"                                             \
											: "=a"(ra)                                             \
											: "0"((ulong)CALLADDR), "b"((ulong)rb), "S"((ulong)rs) \
											: "memory", "cc", "%rcx", "%rdx", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15")
#define DOSYSCALL2(rb, rs, rd, ra) asm volatile("call *%1"                                                             \
												: "=a"(ra)                                                             \
												: "0"((ulong)CALLADDR), "b"((ulong)rb), "S"((ulong)rs), "D"((ulong)rd) \
												: "memory", "cc", "%rcx", "%rdx", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15")

int ttyout = 1;

void bv_nop(void) {
	ulong tmp;

	DOSYSCALL0(SYS_NOP, tmp);
}

void bv_yield(void) {
	ulong tmp;

	DOSYSCALL0(BV_YIELD, tmp);
	return;
}

int bv_net_write(unsigned char *buf, size_t size) {
	ulong tmp;

	DOSYSCALL2(BV_NET_WRITE, buf, size, tmp);
	return (int)tmp;
}

int bv_net_read(unsigned char *buf, size_t size, size_t *rsize) {
	ulong tmp;

	DOSYSCALL2(BV_NET_READ, buf, size, tmp);
	*rsize = tmp;
	return (int)tmp;
}

int bv_block_write(char *buf, int offset, int size) {
	ulong tmp;

	DOSYSCALL2(BV_BLOCK_WRITE, buf, size, tmp);
	return (int)tmp;
}

int bv_block_read(char *buf, int offset, int size) {
	ulong tmp;

	DOSYSCALL2(BV_BLOCK_READ, buf, size, tmp);
	return (int)tmp;
}

int bv_get_time(unsigned long *time) {
	ulong tmp;

	DOSYSCALL1(BV_GET_TIME, time, tmp);
	return (int)tmp;
}

int bv_msgsendint(int desc, int data) {
	ulong tmp;

	DOSYSCALL2(SYS_MSGSENDINT, desc, data, tmp);
	return (int)tmp;
}

int bv_msgopen(const char *name) {
	ulong tmp;

	DOSYSCALL1(SYS_MSGOPEN, name, tmp);
	return (int)tmp;
}

void bv_console_write(const char *buf, unsigned long len) {
	char c;
	while((c = *buf++) != '\0'){
		bv_msgsendint(ttyout, c);
	}
	// bv_msgsendint(ttyout, '\0');
	
	// char b;
	// for (unsigned long i = 0; i < 20; i++) {
	// 	if (*buf == '\0') break;
	// 	b = buf[i];
	// 	bv_msgsendint(ttyout, b);
	// }
}

int bv_set_tls_base(uintptr_t base) {
	int r = -1;
	r = bv_msgsendint(6, (long)base);
	return r;
}