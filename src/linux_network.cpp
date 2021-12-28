#include "crps/linux_network.h"

namespace crps {

bool LinuxNetwork::socket(Protocol p_protocol) {
  // TODO(ahb)
  (void)p_protocol;
  return true;
}

bool LinuxNetwork::bind(const std::string& p_address, uint16_t p_port) {
  // TODO(ahb)
  (void)p_address;
  (void)p_port;
  return true;
}

bool LinuxNetwork::listen() {
  // TODO(ahb)
  return true;
}

bool LinuxNetwork::accept() {
  // TODO(ahb)
  return true;
}

bool LinuxNetwork::connect(const std::string& p_address, uint16_t p_port) {
  // TODO(ahb)
  (void)p_address;
  (void)p_port;
  return true;
}

bool LinuxNetwork::write(void* p_data, size_t p_size) {
  // TODO(ahb)
  (void)p_data;
  (void)p_size;
  return true;
}

ssize_t LinuxNetwork::read(void* p_data, size_t p_max_size) {
  // TODO(ahb)
  (void)p_data;
  (void)p_max_size;
  return 0;  // XXX
}

bool LinuxNetwork::close() {
  // TODO(ahb)
  return true;
}

}  // namespace crps