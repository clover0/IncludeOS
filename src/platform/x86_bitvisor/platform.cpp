#include <os.hpp>
#include <smp>

#include "../../../bitvisor/bitvisor.hpp"

void __arch_poweroff()
{
  asm volatile("cli; hlt");
  for(;;);
}

// returns wall clock time in nanoseconds since the UNIX epoch
uint64_t __arch_system_time() noexcept
{
  return bv_get_time() * 1000;
}

timespec __arch_wall_clock() noexcept
{
  const uint64_t stamp = bv_get_time() * 1000;
  timespec result;
  result.tv_sec = stamp / 1000000000ul;
  result.tv_nsec = stamp % 1000000000ul;
  return result;
}

uint32_t __arch_rand32()
{
  // NOTE: bitvisor does not virtualize TSC
  return bv_get_time() & 0xFFFFFFFF;
}

void __platform_init() {
  // minimal CPU exception handlers already set by bitvisor tender

  // resize up all PER-CPU structures
  for (auto lambda : kernel::smp_global_init) { lambda(); }

  // setup main thread after PER-CPU ctors
  // kernel::setup_main_thread(0);
}

void __arch_reboot() {}
void __arch_enable_legacy_irq(unsigned char) {}
void __arch_disable_legacy_irq(unsigned char) {}
void __arch_subscribe_irq(unsigned char) {}

namespace kernel {
	Fixed_vector<delegate<void()>, 64> smp_global_init(Fixedvector_Init::UNINIT);
}

void SMP::global_lock() noexcept {}
void SMP::global_unlock() noexcept {}
int SMP::cpu_id() noexcept { return 0; }
int SMP::cpu_count() noexcept { return 1; }
void SMP::signal(int) { }
void SMP::add_task(SMP::task_func, int) { };
size_t SMP::early_cpu_total() noexcept { return 1; }

