#ifndef CRPS_LINUX_NETWORK_H
#define CRPS_LINUX_NETWORK_H

#include <netinet/in.h>

#include <map>

#include "crps/network.h"
#include "crps/os.h"

namespace crps {

class LinuxNetwork final : public Network {
 public:
  explicit LinuxNetwork(OS* p_os);
  bool socket(Protocol p_protocol) override;
  bool bind(const std::string& p_address) override;
  bool listen() override;
  bool accept() override;
  bool connect(const std::string& p_address) override;
  bool sendto(const std::string& p_address, void* p_data, size_t p_size) override;
  ssize_t recvfrom(void* p_buffer, size_t p_buffer_size, std::string* p_sender_address) override;
  bool close() override;

 private:
  bool to_sockaddr(const std::string& p_address, sockaddr_in* p_sockaddr);
  bool to_address(const sockaddr_in& p_sockaddr, std::string* p_address);

  int m_socket = -1;
  OS* m_os;
  Protocol m_protocol = Protocol::Invalid;
  std::unordered_map<std::string, sockaddr_in> m_address_to_sockaddr;
  // use std::map instead of std::unordered_map because std::pair does not have a hash function defined
  std::map<std::pair<in_addr_t, in_port_t>, std::string> m_sockaddr_to_address;
};

}  // namespace crps

#endif  // CRPS_LINUX_NETWORK_H