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
// アドレス受け渡しがうまくいかないため配列としておく
struct alignas(4096) page_t {
  char buffer[4096];
};
static std::array<page_t, 1024> machine_pool;
uint32_t __multiboot_addr = 0;
// extern "C" void pre_initialize_tls();
extern int ttyout;
extern int ctnr_num;

static std::vector<int> v;


// extern "C"
void kernel_start(const struct bv_start_info *si) {
  bv_nop(); //2
  free_mem_begin = si->heap_start;
  mem_size = si->heap_size;

  // __init_sanity_checks();
  
  // Preserve symbols from the ELF binary
  const size_t len = _move_symbols(free_mem_begin);
  // const size_t len = _move_symbols(machine_pool.data());
  free_mem_begin += len;
  mem_size -= len;
  printf("move symbols\n");

  // Ze machine
  // __machine = os::Machine::create((void*)free_mem_begin, mem_size);
  __machine = os::Machine::create(machine_pool.data(), sizeof(machine_pool));
  printf("machine create\n");

  _init_elf_parser();

  // Begin portable HAL initialization
  __machine->init();
  printf("machine init\n");

  // Initialize system calls
  // _init_syscalls();

  printf("init syscall\n");

  x86::init_libc((uint32_t) (uintptr_t) temp_cmdline, 0);
}

extern "C" int bv_main_start() {
  static struct bv_start_info si;
  extern char _stext[], _etext[], _erodata[], _end[];
  uint64_t _mem_size = 0x01E00000; // on bv process size 30MB
  static uint64_t heap_start;
  int ukld, num, r;

  heap_start = ((uint64_t)&_end + PAGE_SIZE - 1) & PAGE_MASK;

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
  si.heap_start = heap_start;
  // si.heap_size = _mem_size - heap_start;
  si.heap_size = _mem_size;

  printf("_end: 0x%llx\n", (unsigned long long)_end);
  printf("heap start: 0x%llx\n", (unsigned long long)si.heap_start);
  printf("heap size: 0x%llx\n", (unsigned long long)si.heap_size);

  // setup container id
  num = bv_msgsendint(ukld, 5); // 5 is to get container id
  printf("container %d start\n", num);
  ctnr_num = num;

  kernel_start(&si);
  // pre_initialize_tls();

  return 0;
}