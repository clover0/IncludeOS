
#include <common>
#include <hw/pci_device.hpp>
#include <net/buffer_store.hpp>
#include <net/link_layer.hpp>
#include <net/ethernet/ethernet.hpp>
#include <delegate>
#include <deque>
#include <statman>

#include "../../bitvisor/bitvisor.hpp"

class BVNet : public net::Link_layer<net::Ethernet> {
public:
  using Link          = net::Link_layer<net::Ethernet>;
  using Link_protocol = Link::Protocol;

  static std::unique_ptr<Nic> new_instance()
  {
    return std::make_unique<BVNet>();
  }

  /** Human readable name. */
  const char* driver_name() const override;

  /** Mac address. */
  const MAC::Addr& mac() const noexcept override {
    return mac_addr;
  }

  uint16_t MTU() const noexcept override
  { return 1500; }

  uint16_t packet_len() const noexcept {
    return sizeof(net::ethernet::Header) + MTU();
  }

  net::downstream create_physical_downstream() override
  { return {this, &BVNet::transmit}; }

  /** Linklayer input. Hooks into IP-stack bottom, w.DOWNSTREAM data.*/
  void transmit(net::Packet_ptr pckt);

  /** Constructor. @param pcidev an initialized PCI device. */
  BVNet();

  /** Space available in the transmit queue, in packets */
  size_t transmit_queue_available() override {
    return 1000; // any big random number for now
  }

  net::Packet_ptr create_packet(int) override;

  auto& bufstore() noexcept { return bufstore_; }

  void move_to_this_cpu() override {};

  void deactivate() override;

  void flush() override {};

  void poll() override;

private:
  MAC::Addr mac_addr;
  std::unique_ptr<net::Packet> recv_packet();
  /** Stats */
  uint64_t& packets_rx_;
  uint64_t& packets_tx_;

  net::BufferStore bufstore_;

};
