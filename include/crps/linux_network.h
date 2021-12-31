#ifndef CRPS_LINUX_NETWORK_H
#define CRPS_LINUX_NETWORK_H

#include "crps/network.h"
#include "crps/os.h"

namespace crps {

class LinuxNetwork final : public Network {
 public:
  explicit LinuxNetwork(OS* p_os);
  bool socket(Protocol p_protocol) override;
  bool bind(const std::string& p_address, uint16_t p_port) override;
  bool listen() override;
  bool accept() override;
  bool connect(const std::string& p_address, uint16_t p_port) override;
  bool sendto(const std::string& p_address, void* p_data, size_t p_size) override;
  ssize_t recvfrom(void* p_buffer, size_t p_buffer_size, std::string* p_sender) override;
  bool close() override;

 private:
  int m_socket = -1;
  OS* m_os;
  Protocol m_protocol = Protocol::Invalid;
};

}  // namespace crps

#endif  // CRPS_LINUX_NETWORK_H