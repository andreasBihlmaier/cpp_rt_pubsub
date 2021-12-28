#ifndef CRPS_LINUX_NETWORK_H
#define CRPS_LINUX_NETWORK_H

#include "crps/network.h"

namespace crps {

class LinuxNetwork final : public Network {
 public:
  bool socket(Protocol p_protocol) override;
  bool bind(const std::string& p_address, uint16_t p_port) override;
  bool listen() override;
  bool accept() override;
  bool connect(const std::string& p_address, uint16_t p_port) override;
  bool write(void* p_data, size_t p_size) override;
  ssize_t read(void* p_data, size_t p_max_size) override;
  bool close() override;
};

}  // namespace crps

#endif  // CRPS_LINUX_NETWORK_H