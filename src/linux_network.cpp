#include "crps/linux_network.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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

bool LinuxNetwork::bind(const std::string& p_address) {
  sockaddr_in bind_address{};
  if (!to_sockaddr(p_address, &bind_address)) {
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

bool LinuxNetwork::connect(const std::string& p_address) {
  sockaddr_in connect_address{};
  if (!to_sockaddr(p_address, &connect_address)) {
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

bool LinuxNetwork::sendto(const std::string& p_address, const void* p_data, size_t p_size) {
  sockaddr_in receiver_sockaddr{};
  if (!to_sockaddr(p_address, &receiver_sockaddr)) {
    m_os->logger().error() << "to_sockaddr_in() failed: " << std::strerror(errno) << "\n";
    return false;
  }
  ssize_t bytes_written =
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      ::sendto(m_socket, p_data, p_size, 0, reinterpret_cast<sockaddr*>(&receiver_sockaddr), sizeof(receiver_sockaddr));
  if (bytes_written < 0) {
    m_os->logger().error() << "sendto() failed: " << std::strerror(errno) << "\n";
    return false;
  }
  return static_cast<size_t>(bytes_written) == p_size;
}

ssize_t LinuxNetwork::recvfrom(void* p_buffer, size_t p_buffer_size, bool block, std::string* p_sender_address) {
  int flags{0};
  if (!block) {
    flags = MSG_DONTWAIT;
  }
  sockaddr_in sender_sockaddr{};
  socklen_t sender_sockaddr_size = sizeof(sender_sockaddr);
  ssize_t bytes_received =
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      ::recvfrom(m_socket, p_buffer, p_buffer_size, flags, reinterpret_cast<sockaddr*>(&sender_sockaddr),
                 &sender_sockaddr_size);
  if (bytes_received < 0) {
    if (block || (errno != EAGAIN && errno != EWOULDBLOCK)) {
      m_os->logger().error() << "recvfrom() failed: " << std::strerror(errno) << "\n";
      return -1;
    }
  }
  if (p_sender_address != nullptr) {
    if (!to_address(sender_sockaddr, p_sender_address)) {
      return -1;
    }
  }
  return bytes_received;
}

bool LinuxNetwork::close() {
  // TODO(ahb)
  m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
  return false;
}

bool LinuxNetwork::to_sockaddr(const std::string& p_address, sockaddr_in* p_sockaddr) {
  if (m_address_to_sockaddr.find(p_address) == m_address_to_sockaddr.end()) {
    sockaddr_in new_sockaddr{};
    const int protocol_family = AF_INET;  // IPv4

    auto address_delimiter = p_address.find(':');
    std::string host = p_address.substr(0, address_delimiter);
    uint16_t port = std::stoi(p_address.substr(address_delimiter + 1));

    if (inet_pton(protocol_family, host.c_str(), &new_sockaddr.sin_addr) != 1) {
      return false;
    }
    new_sockaddr.sin_family = protocol_family;
    new_sockaddr.sin_port = htons(port);
    m_address_to_sockaddr[p_address] = new_sockaddr;
    CRPS_LOGGER_DEBUG(m_os, << "Adding " << p_address << " to m_address_to_sockaddr.\n");
  }
  *p_sockaddr = m_address_to_sockaddr[p_address];

  return true;
}

bool LinuxNetwork::to_address(const sockaddr_in& p_sockaddr, std::string* p_address) {
  auto sockaddr_key = std::make_pair(p_sockaddr.sin_addr.s_addr, p_sockaddr.sin_port);
  if (m_sockaddr_to_address.find(sockaddr_key) == m_sockaddr_to_address.end()) {
    std::array<char, INET_ADDRSTRLEN> address_buffer{};
    if (inet_ntop(p_sockaddr.sin_family, &p_sockaddr.sin_addr, address_buffer.data(), INET_ADDRSTRLEN) == nullptr) {
      m_os->logger().error() << "inet_ntop() failed: " << std::strerror(errno) << "\n";
      return false;
    }
    m_sockaddr_to_address[sockaddr_key] =
        std::string(address_buffer.data()) + ":" + std::to_string(ntohs(p_sockaddr.sin_port));
    CRPS_LOGGER_DEBUG(m_os, << "Adding " << m_sockaddr_to_address[sockaddr_key] << " to m_sockaddr_to_address.\n");
  }
  *p_address = m_sockaddr_to_address[sockaddr_key];
  return true;
}

}  // namespace crps