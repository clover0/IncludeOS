#include "bitvisor.hpp"

typedef unsigned long ulong;

#define CALLADDR		0x3FFFF000
#define SYS_NOP			1

#define BV_YIELD			16
#define BV_NET_WRITE		17
#define BV_NET_READ			18
#define BV_BLOCK_WRITE		19
#define BV_BLOCK_READ		20
#define BV_GET_TIME			21

// We can use only x86_64 arch
#	define DOSYSCALL0(rb, ra) asm volatile \
		("call *%1" : "=a" (ra) \
			    : "0" ((ulong)CALLADDR), "b" ((ulong)rb) \
			    : "memory", "cc", "%rcx", "%rdx" \
			    , "%r8", "%r9", "%r10", "%r11", "%r12", "%r13" \
			    , "%r14", "%r15")
#	define DOSYSCALL1(rb, rs, ra) asm volatile \
		("call *%1" : "=a" (ra) \
			    : "0" ((ulong)CALLADDR), "b" ((ulong)rb) \
			    , "S" ((ulong)rs) \
			    : "memory", "cc", "%rcx", "%rdx" \
			    , "%r8", "%r9", "%r10", "%r11", "%r12", "%r13" \
			    , "%r14", "%r15")
#	define DOSYSCALL2(rb, rs, rd, ra) asm volatile \
		("call *%1" : "=a" (ra) \
			    : "0" ((ulong)CALLADDR), "b" ((ulong)rb) \
			    , "S" ((ulong)rs), "D" ((ulong)rd) \
			    : "memory", "cc", "%rcx", "%rdx" \
			    , "%r8", "%r9", "%r10", "%r11", "%r12", "%r13" \
			    , "%r14", "%r15")

void
bv_nop (void)
{
	ulong tmp;

	DOSYSCALL0 (SYS_NOP, tmp);
}

void
bv_yield(void)
{
	ulong tmp;

	DOSYSCALL0 (BV_YIELD, tmp);
	return;
}

int
bv_net_write (char *buf, int size)
{
	ulong tmp;

	DOSYSCALL2 (BV_NET_WRITE, buf, size, tmp);
	return (int)tmp;
}

int
bv_net_read (char *buf, int size)
{
	ulong tmp;

	DOSYSCALL2 (BV_NET_READ, buf, size, tmp);
	return (int)tmp;
}

int
bv_block_write (char *buf, int offset, int size)
{
	ulong tmp;

	DOSYSCALL2 (BV_BLOCK_WRITE, buf, size, tmp);
	return (int)tmp;
}

int
bv_block_read (char *buf, int offset, int size)
{
	ulong tmp;

	DOSYSCALL2 (BV_BLOCK_READ, buf, size, tmp);
	return (int)tmp;
}

int
bv_get_time (unsigned long *time)
{
	ulong tmp;

	DOSYSCALL1 (BV_GET_TIME, time, tmp);
	return (int)tmp;
}
