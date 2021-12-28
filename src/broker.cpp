#include "crps/broker.h"

namespace crps {

Broker::Broker(std::string p_listen_address, Network* p_network)
    : m_listen_address(std::move(p_listen_address)), m_network(p_network) {
  // TODO(ahb) prevent clang-diagnostic-unused-private-field -> remove
  (void)m_listen_address;
  (void)m_network;
}

void Broker::start() {
  // TODO(ahb)
}

}  // namespace crps