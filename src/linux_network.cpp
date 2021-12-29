#include "crps/linux_network.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstring>

namespace crps {

LinuxNetwork::LinuxNetwork(OS* p_os) : m_os(p_os) {
}

bool LinuxNetwork::socket(Protocol p_protocol) {
  m_protocol = p_protocol;

  int socket_domain{0};
  int socket_type{0};
  int socket_protocol{0};

  if (p_protocol == Protocol::UDP) {
    socket_domain = AF_INET;
    socket_type = SOCK_DGRAM;
    socket_protocol = 0;
  } else {
    // TODO(ahb)
    m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
    return false;
  }

  if ((m_socket = ::socket(socket_domain, socket_type, socket_protocol)) < 0) {
    m_os->logger().error() << "socket() failed: " << std::strerror(errno) << "\n";
    return false;
  }

  // Allow quick reuse of port
  const int sockopt = 1;
  if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) != 0) {
    m_os->logger().error() << "setsockopt() failed: " << std::strerror(errno) << "\n";
  }

  return true;
}

bool LinuxNetwork::bind(const std::string& p_address, uint16_t p_port) {
  sockaddr_in bind_address{};

  const int protocol_family = AF_INET;  // IPv4

  if (inet_pton(protocol_family, p_address.c_str(), &bind_address) != 1) {
    m_os->logger().error() << "inet_pton() failed: " << std::strerror(errno) << "\n";
    return false;
  }
  bind_address.sin_family = protocol_family;
  bind_address.sin_port = htons(p_port);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (::bind(m_socket, reinterpret_cast<sockaddr*>(&bind_address), sizeof(bind_address)) != 0) {
    m_os->logger().error() << "bind() failed: " << std::strerror(errno) << "\n";
    return false;
  }

  return true;
}

bool LinuxNetwork::listen() {
  if (m_protocol == Protocol::TCP) {
    // TODO(ahb)
    m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
    return false;
  }

  // Nothing to do for other protocols
  return true;
}

bool LinuxNetwork::accept() {
  if (m_protocol == Protocol::TCP) {
    // TODO(ahb)
    m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
    return false;
  }

  // Nothing to do for other protocols
  return true;
}

bool LinuxNetwork::connect(const std::string& p_address, uint16_t p_port) {
  // TODO(ahb)
  (void)p_address;
  (void)p_port;
  m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
  return false;
}

bool LinuxNetwork::write(void* p_data, size_t p_size) {
  // TODO(ahb)
  (void)p_data;
  (void)p_size;
  m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
  return false;
}

ssize_t LinuxNetwork::read(void* p_data, size_t p_max_size) {
  // TODO(ahb)
  (void)p_data;
  (void)p_max_size;
  m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
  return -1;
}

bool LinuxNetwork::close() {
  // TODO(ahb)
  m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
  return false;
}

}  // namespace crps