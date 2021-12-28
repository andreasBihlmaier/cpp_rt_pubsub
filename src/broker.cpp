#include "crps/broker.h"

namespace crps {

Broker::Broker(std::string p_listen_address) : m_listen_address(std::move(p_listen_address)) {
  // TODO(ahb) prevent clang-diagnostic-unused-private-field -> remove
  (void)m_listen_address;
}

void Broker::start() {
  // TODO(ahb)
}

}  // namespace crps