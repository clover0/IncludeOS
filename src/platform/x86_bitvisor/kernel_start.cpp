#include "../x86_pc/init_libc.hpp"
#include <info>
#include <kernel.hpp>
#include <kprint>
#include <vector>

// extern "C" {
// これどうにかしたい
#include "../../../bitvisor/bitvisor.hpp"
// }

extern "C" {
void __init_sanity_checks();
uintptr_t _move_symbols(uintptr_t loc);
void _init_syscalls();
void _init_elf_parser();
}

// TODO: move
#define PAGE_SIZE 4096
#define PAGE_MASK ~(0xfff)

static os::Machine *__machine = nullptr;
os::Machine &os::machine() noexcept {
  Expects(__machine != nullptr);
  return *__machine;
}

static char temp_cmdline[1024];
static uintptr_t mem_size = 0;
static uintptr_t free_mem_begin;
struct alignas(4096) page_t {
  char buffer[4096];
};
static std::array<page_t, 1024> machine_pool;
uint32_t __multiboot_addr = 0;
extern "C" void pre_initialize_tls();
extern int ttyout;
extern int ctnr_num;
static std::vector<int> v;
void kernel_start();

extern "C" int bv_main_start() {
  extern char _stext[], _etext[], _erodata[], _end[];
  // on bitvisor, 1 process = program memsize + config size(heap=50MB)
  uint64_t _mem_size = 0x03200000; // as 50MB
  static uint64_t heap_start;
  int ukld, num, r;
  unsigned long rsp, rbp;

  heap_start = ((uint64_t)&_end + PAGE_SIZE - 1) & PAGE_MASK;

  // setup tty
  ttyout = bv_msgopen("ttyout");
  if (ttyout < 0) {
    return 1;
  }

  // setup jjjukl handler
  ukld = bv_msgopen("ukl");
  if (ttyout < 0) {
    printf("cant open ukl\n");
    return 1;
  }

  // set tls
  bv_msgsendint(ukld, 6);

  // setup heap
  free_mem_begin = heap_start;
  mem_size = _mem_size - heap_start;
  // si.heap_size = _mem_size;

  printf("_end: 0x%llx\n", (unsigned long long)_end);
  printf("heap start: 0x%llx\n", (unsigned long long)free_mem_begin);
  printf("heap size: 0x%llx\n", (unsigned long long)mem_size);

  // setup container id
  num = bv_msgsendint(ukld, 5); // 5 is to get container id
  printf("container j%d start\n", num);
  ctnr_num = num;

  asm volatile ("mov %%rsp,%0"
		      : "=r" (rsp));
  printf("includeos RSP(before kenel_start): %x\n", rsp);
  asm volatile ("mov %%rbp,%0"
		      : "=r" (rbp));
  printf("includeos RBP(before kenel_start): %x\n", rbp);


  // pre_initialize_tls();
  kernel_start();

  return 0;
}

// extern "C" 
void kernel_start() {
  bv_nop(); //2

  __init_sanity_checks();

  // Preserve symbols from the ELF binary
  // const size_t len = _move_symbols(free_mem_begin);
  // free_mem_begin += len;
  // mem_size -= len;
  printf("move symbols\n");

  // Initialize .bss
  extern char _BSS_START_, _BSS_END_;
  __builtin_memset(&_BSS_START_, 0, &_BSS_END_ - &_BSS_START_);

  // Ze machine
  // __machine = os::Machine::create((void*)free_mem_begin, mem_size);
  __machine = os::Machine::create(machine_pool.data(), sizeof(machine_pool));
  printf("machine create\n");

  _init_elf_parser();

  // Begin portable HAL initialization
  __machine->init();
  // kernel::init_heap((uintptr_t)free_mem_begin, (uintptr_t)free_mem_begin + mem_size);
  printf("machine init\n");

  // printf("vector address ::%p::\n", &v);
  // v.push_back(5);
  // printf("vector pushe 5 \n");
  // v.push_back(2);
  // printf("vector pushed ! \n");
  // printf("vector[1] ::%p, %d::\n", &v[1], v[1]);
  // printf("vector[2] ::%p::\n", &v[2]);
  // printf("test vec size %d \n", v.size());

  // Initialize system calls
  _init_syscalls();

  printf("init syscall\n");

  x86::init_libc((uint32_t)(uintptr_t)temp_cmdline, 0);
}
