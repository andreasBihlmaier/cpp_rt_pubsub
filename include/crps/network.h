#ifndef CRPS_NETWORK_H
#define CRPS_NETWORK_H

#include <cstdint>
#include <string>

namespace crps {

class Network {
 public:
  enum class Protocol {
    Invalid = 0,
    UDP = 1,
    TCP = 2,
  };

  virtual bool socket(Protocol p_protocol) = 0;
  virtual bool bind(const std::string& p_address, uint16_t p_port) = 0;
  virtual bool listen() = 0;
  virtual bool accept() = 0;
  virtual bool connect(const std::string& p_address, uint16_t p_port) = 0;
  virtual bool sendto(const std::string& p_address, void* p_data, size_t p_size) = 0;
  virtual ssize_t recvfrom(void* p_buffer, size_t p_buffer_size, std::string* p_sender) = 0;
  virtual bool close() = 0;
};

}  // namespace crps

#endif  // CRPS_NETWORK_H