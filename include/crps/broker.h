#ifndef CRPS_BROKER_H
#define CRPS_BROKER_H

#include <string>

#include "crps/message.h"
#include "crps/topic.h"

namespace crps {

class Broker {
 private:
  std::string m_listen_address;

 public:
  explicit Broker(std::string p_listen_address);
  void start();
};

}  // namespace crps

#endif  // CRPS_BROKER_H