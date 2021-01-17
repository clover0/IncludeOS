
#include "bvnet.hpp"
#include <net/packet.hpp>
#include <hw/pci.hpp>
#include <cstdio>
#include <cstring>

#include "../../bitvisor/bitvisor.hpp"

static const uint32_t NUM_BUFFERS = 1024;
using namespace net;

const char* BVNet::driver_name() const { return "BVNet"; }

BVNet::BVNet()
  : Link(Link_protocol{{this, &BVNet::transmit}, mac()}),
    packets_rx_{Statman::get().create(Stat::UINT64, device_name() + ".packets_rx").get_uint64()},
    packets_tx_{Statman::get().create(Stat::UINT64, device_name() + ".packets_tx").get_uint64()},
    bufstore_{NUM_BUFFERS, 2048u} // don't change this
{
  INFO("BVNet", "Driver initializing");
  struct bv_net_info ni;
  // bv_net_info(&ni);
  // mac_addr = MAC::Addr(ni.mac_address[0], ni.mac_address[1], ni.mac_address[2],
  //                      ni.mac_address[3], ni.mac_address[4], ni.mac_address[5]);
  // mac_addr = MAC::Addr(0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc);
  // TODO: 
  mac_addr = MAC::Addr(0x00, 0x0c, 0x29, 0xeb, 0xe3, 0xd6); // BV MacPC NIC
  // mac_addr = MAC::Addr(0x00, 0x1b, 0x41, 0x01, 0x0c, 0x84); // BV UbuntuPC NIC
}

void BVNet::transmit(net::Packet_ptr pckt)
{
  net::Packet_ptr tail = std::move(pckt);

  // Transmit all we can directly
  while (tail) {
    // next in line
    auto next = tail->detach_tail();
    // write data to networkn
    bv_net_write(tail->buf(), tail->size());
    // set tail to next, releasing tail
    tail = std::move(next);
    // Stat increase packets transmitted
    packets_tx_++;
  }

  // Buffer the rest
  if (UNLIKELY(tail)) {
    INFO("bvnet", "Could not send all packets..\n");
  }
}

net::Packet_ptr BVNet::create_packet(int link_offset)
{
  auto* pckt = (net::Packet*) bufstore().get_buffer();

  new (pckt) net::Packet(link_offset, 0, packet_len(), &bufstore());
  return net::Packet_ptr(pckt);
}
net::Packet_ptr BVNet::recv_packet()
{
  auto* pckt = (net::Packet*) bufstore().get_buffer();
  new (pckt) net::Packet(0, MTU(), packet_len(), &bufstore());
  // Populate the packet buffer with new packet, if any
  size_t size = packet_len();
  if (bv_net_read(pckt->buf(), size, &size) == 0) {
    // Adjust packet size to match received data
    if (size) {
      pckt->set_data_end(size);
      return net::Packet_ptr(pckt);
    }
  }
  bufstore().release(pckt);
  return nullptr;
}

void BVNet::poll()
{
  auto pckt_ptr = recv_packet();

  if (LIKELY(pckt_ptr != nullptr)) {
    Link::receive(std::move(pckt_ptr));
  }
}

void BVNet::deactivate()
{
  INFO("BVNet", "deactivate");
}

#include <kernel/bitvisor_manager.hpp>

struct Autoreg_bvnet {
  Autoreg_bvnet() {
    BitVisor_manager::register_net(&BVNet::new_instance);
  }
} autoreg_bvnet;
