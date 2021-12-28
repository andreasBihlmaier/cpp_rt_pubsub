#ifndef CRPS_BROKER_H
#define CRPS_BROKER_H

#include <memory>
#include <string>

#include "crps/message.h"
#include "crps/network.h"
#include "crps/topic.h"

namespace crps {

class Broker {
 private:
  std::string m_listen_address;
  Network* m_network;

 public:
  explicit Broker(std::string p_listen_address, Network* p_network);
  void start();
};

}  // namespace crps

#endif  // CRPS_BROKER_H