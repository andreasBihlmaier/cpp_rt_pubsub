#ifndef CRPS_LINUX_NETWORK_H
#define CRPS_LINUX_NETWORK_H

#include "crps/network.h"
#include "crps/os.h"

namespace crps {

class LinuxNetwork final : public Network {
 private:
  int m_socket = -1;
  OS* m_os;
  Protocol m_protocol = Protocol::Invalid;

 public:
  explicit LinuxNetwork(OS* p_os);
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