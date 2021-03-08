
#ifndef KERNEL_BitVisor_MANAGER_HPP
#define KERNEL_BitVisor_MANAGER_HPP

#include <memory>
#include <delegate>
#include <hw/nic.hpp>
#include <hw/block_device.hpp>

class BitVisor_manager {
public:
  using Nic_ptr = std::unique_ptr<hw::Nic>;
  using Blk_ptr = std::unique_ptr<hw::Block_device>;

  static void register_net(delegate<Nic_ptr()>);
  static void register_blk(delegate<Blk_ptr()>);

  static void init();
}; //< class BitVisor_manager

#endif //< KERNEL_BitVisor_MANAGER_HPP
