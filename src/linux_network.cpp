#include "crps/linux_network.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

namespace crps {

bool to_sockaddr(const std::string& p_address, uint16_t p_port, sockaddr_in* p_sock_address) {
  const int protocol_family = AF_INET;  // IPv4

  if (inet_pton(protocol_family, p_address.c_str(), p_sock_address) != 1) {
    return false;
  }
  p_sock_address->sin_family = protocol_family;
  p_sock_address->sin_port = htons(p_port);

  return true;
}

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
  if (!to_sockaddr(p_address, p_port, &bind_address)) {
    m_os->logger().error() << "to_sockaddr_in() failed: " << std::strerror(errno) << "\n";
    return false;
  }

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
  sockaddr_in connect_address{};
  if (!to_sockaddr(p_address, p_port, &connect_address)) {
    m_os->logger().error() << "to_sockaddr_in() failed: " << std::strerror(errno) << "\n";
    return false;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (::connect(m_socket, reinterpret_cast<sockaddr*>(&connect_address), sizeof(connect_address)) != 0) {
    m_os->logger().error() << "connect() failed: " << std::strerror(errno) << "\n";
    return false;
  }

  return true;
}

bool LinuxNetwork::write(void* p_data, size_t p_size) {
  ssize_t bytes_written = ::write(m_socket, p_data, p_size);
  if (bytes_written < 0) {
    m_os->logger().error() << "write() failed: " << std::strerror(errno) << "\n";
    return false;
  }
  return static_cast<size_t>(bytes_written) == p_size;
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