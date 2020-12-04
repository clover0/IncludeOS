#include <kernel.hpp>
#include "../x86_pc/init_libc.hpp"
#include <kprint>
#include <info>

// extern "C" {
// #include <bitvisor/bitvisor.h>
// }

// extern "C" {
//   void __init_sanity_checks();
//   uintptr_t _move_symbols(uintptr_t loc);
//   void _init_syscalls();
//   void _init_elf_parser();
// }

static os::Machine* __machine = nullptr;
os::Machine& os::machine() noexcept {
  Expects(__machine != nullptr);
  return *__machine;
}

static char temp_cmdline[1024];
static uintptr_t mem_size = 0;
static uintptr_t free_mem_begin;
uint32_t __multiboot_addr = 0;
extern "C" void pre_initialize_tls();

extern "C"
int bitvisor_app_main(const struct bitvisor_start_info *si)
{
  // si is stored at 0x6000 by bitvisor tender which is used by includeos. Move it fast.
  // strncpy(temp_cmdline, si->cmdline, sizeof(temp_cmdline)-1);
  // temp_cmdline[sizeof(temp_cmdline)-1] = 0;
  // free_mem_begin = si->heap_start;
  // mem_size = si->heap_size;

  // pre_initialize_tls();
  return 0;
}

extern "C"
void kernel_start()
{
  // generate checksums of read-only areas etc.
  // __init_sanity_checks();
  printf("start kernel\n");

  // Preserve symbols from the ELF binary
  // const size_t len = _move_symbols(free_mem_begin);
  // free_mem_begin += len;
  // mem_size -= len;

  // Ze machine
  // __machine = os::Machine::create((void*)free_mem_begin, mem_size);

  // _init_elf_parser();

  // Begin portable HAL initialization
  // __machine->init();

  // Initialize system calls
  // _init_syscalls();

  // x86::init_libc((uint32_t) (uintptr_t) temp_cmdline, 0);
}
