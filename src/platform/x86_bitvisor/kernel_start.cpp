#include "../x86_pc/init_libc.hpp"
#include <info>
#include <kernel.hpp>
#include <kprint>

// extern "C" {
// これどうにかしたい
#include "../../../bitvisor/bitvisor.hpp"
// }

extern "C" {
//   void __init_sanity_checks();
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

// static char temp_cmdline[1024];
static uintptr_t mem_size = 0;
static uintptr_t free_mem_begin;
uint32_t __multiboot_addr = 0;
// extern "C" void pre_initialize_tls();
extern int ttyout;

// extern "C"
void kernel_start(const struct bv_start_info *si) {
  bv_nop(); //2
  // free_mem_begin = si->heap_start;
  free_mem_begin = 1;
  mem_size = si->heap_size;

  // printf("kernel_start: heap >= 0x%llx < stack < 0x%llx\n",
        //  (unsigned long long)free_mem_begin, (unsigned long long)mem_size);

  // Preserve symbols from the ELF binary
  // const size_t len = _move_symbols(free_mem_begin);
  // free_mem_begin += len;
  // mem_size -= len;

  // Ze machine
  // __machine = os::Machine::create((void*)free_mem_begin, mem_size);

  printf("machine\n");
  // _init_elf_parser();
  // _init_elf_parser();
  printf("perser %d\n", 1);

  // Begin portable HAL initialization
  // __machine->init();
  printf("init\n");

  // Initialize system calls
  // _init_syscalls();
  printf("init syscall\n");

  kernel::start(); // 本来は大分あと
  printf("start kernel\n");

  // printf("hello unikernle!\n");
  // generate checksums of read-only areas etc.
  // __init_sanity_checks();
  kernel::post_start();
  printf("kernel post start");

  // x86::init_libc((uint32_t) (uintptr_t) temp_cmdline, 0);
}

extern "C" int bv_main_start() {
  static struct bv_start_info si;
  extern char _stext[], _etext[], _erodata[], _end[];
  // uint64_t _mem_size = 0x0100000 * 5;
  uint64_t _mem_size = 0x0100000;
  static uint64_t heap_start;
  int ukld;

  // heap_start = ((uint64_t)&_end + PAGE_SIZE - 1) & PAGE_MASK;

  // setup tty
  ttyout = bv_msgopen("ttyout");
  if (ttyout < 0) {
    return 1;
  }

  // setup ukl handler
  ukld = bv_msgopen("ukl");
  if (ttyout < 0) {
    printf("cant open ukl\n");
    return 1;
  }

  // setup heap
  heap_start = bv_msgsendint(ukld, 4); // 4 is get heap address

  si.heap_start = heap_start;
  si.heap_size = _mem_size - heap_start;

  // これはエラー
  printf("heap start: 0x%llx \n", (unsigned long long)heap_start);

  // printf("_end: 0x%llx \n", (unsigned long long)_end);
  // printf("heap >= 0x%llx < stack < 0x%llx\n",
  //  (unsigned long long)si.heap_start, (unsigned long long)si.heap_size);

  // heap

  kernel_start(&si);
  // pre_initialize_tls();

  return 0;
}
