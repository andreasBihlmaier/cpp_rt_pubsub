#include "crps/broker.h"

#include <chrono>
#include <thread>

namespace crps {

Broker::Broker(std::string p_listen_address, Network* p_network, Network::Protocol p_protocol, int16_t p_port)
    : m_listen_address(std::move(p_listen_address)), m_network(p_network), m_protocol(p_protocol), m_port(p_port) {
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

  for (;;) {
    std::this_thread::sleep_for(100ms);
    (void)m_network;
  }
}

}  // namespace crps