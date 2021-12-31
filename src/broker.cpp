#include "crps/broker.h"

#include <chrono>
#include <nlohmann/json.hpp>
#include <thread>

namespace crps {

Broker::Broker(std::string p_listen_address, OS* p_os, Network* p_network, Network::Protocol p_protocol, int16_t p_port)
    : m_listen_address(std::move(p_listen_address)),
      m_os(p_os),
      m_network(p_network),
      m_protocol(p_protocol),
      m_port(p_port) {
}

bool Broker::start() {
  if (!m_network->socket(m_protocol)) {
    return false;
  }

  if (!m_network->bind(m_listen_address, m_port)) {
    return false;
  }

  if (!m_network->listen()) {
    return false;
  }

  return true;
}

void Broker::spin() {
  using namespace std::chrono_literals;
  const size_t buffer_size = 64 * 1024;  // 64K is maximum UDP datagram size
  std::array<unsigned char, buffer_size> buffer{};

  for (;;) {
    std::string client;
    ssize_t bytes_received_signed = m_network->recvfrom(buffer.data(), buffer_size, &client);
    m_os->logger().debug() << "received " << bytes_received_signed << " bytes from " << client << "\n";
    if (bytes_received_signed < 0) {
      m_os->logger().error() << "recvfrom() failed. This should not happen! Continuing.\n";
      continue;
    }
    auto bytes_received = static_cast<size_t>(bytes_received_signed);
    if (bytes_received < bp_header_size) {
      m_os->logger().error() << "Received packet shorter than BpHeader from " << client
                             << ". This indicates a serious issue! Ignoring packet and continuing.\n";
      continue;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* bp_header = reinterpret_cast<BpHeader*>(buffer.data());
    if (!verify_and_update_client_counter(client, bp_header->counter)) {
      m_os->logger().error() << "BpHeader::counter from " << client
                             << " is wrong. This indicates lost packets! Ignoring packet and continuing.\n";
      continue;
    }
    if (bp_header->type == BpType::Control) {
      m_os->logger().debug() << "-> BpType::Control\n";
    } else if (bp_header->type == BpType::Data) {
      // TODO(ahb)
      m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
      break;
    }
  }
}

bool Broker::verify_and_update_client_counter(const std::string& p_client, BpCounterType p_counter) {
  if (m_client_counters.find(p_client) == m_client_counters.end()) {  // new client
    m_client_counters[p_client] = p_counter;
  }
  return m_client_counters[p_client] == p_counter;
}

}  // namespace crps