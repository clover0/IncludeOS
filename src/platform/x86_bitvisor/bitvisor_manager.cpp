
#include <cassert>
#include <common>
#include <delegate>
#include <vector>

#include <kernel/bitvisor_manager.hpp>
#include <stdexcept>
#include <hal/machine.hpp>

static std::vector<delegate<BitVisor_manager::Nic_ptr()>> nics;
static std::vector<delegate<BitVisor_manager::Blk_ptr()>> blks;

void BitVisor_manager::register_net(delegate<Nic_ptr()> func)
{
  nics.push_back(func);
}
void BitVisor_manager::register_blk(delegate<Blk_ptr()> func)
{
  blks.push_back(func);
}

void BitVisor_manager::init() {
  INFO("bv", "Looking for bv devices");

  for (auto nic : nics)
    os::machine().add<hw::Nic> (nic());
  // for (auto blk : blks) // TODO
    // os::machine().add<hw::Block_device> (blk());
}
