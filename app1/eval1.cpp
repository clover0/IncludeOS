#include <os>
#include "../bitvisor/bitvisor.hpp"

void Service::start() {
  printf("Booted at monotonic_us=%ld \n", bv_get_time());
}
