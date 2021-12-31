#ifndef CRPS_BROKER_H
#define CRPS_BROKER_H

#include <memory>
#include <string>

#include "crps/broker_protocol.h"
#include "crps/defaults.h"
#include "crps/message.h"
#include "crps/network.h"
#include "crps/os.h"
#include "crps/topic.h"

namespace crps {

class Broker {
 private:
  std::string m_listen_address;
  OS* m_os;
  Network* m_network;
  Network::Protocol m_protocol;
  int16_t m_port;

 public:
  explicit Broker(std::string p_listen_address, OS* p_os, Network* p_network,
                  Network::Protocol p_protocol = Network::Protocol::UDP, int16_t p_port = broker_default_port);
  bool start();
  void spin();
};

}  // namespace crps

#endif  // CRPS_BROKER_H