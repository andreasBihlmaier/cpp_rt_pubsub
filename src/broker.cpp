#include "crps/broker.h"

#include <chrono>
#include <nlohmann/json.hpp>
#include <thread>

namespace crps {

Broker::Broker(std::string p_listen_address, OS* p_os, Network* p_network, Network::Protocol p_protocol, int16_t p_port)
    : m_listen_address(std::move(p_listen_address) + ":" + std::to_string(p_port)),
      m_os(p_os),
      m_network(p_network),
      m_protocol(p_protocol) {
}

bool Broker::start() {
  if (!m_network->socket(m_protocol)) {
    return false;
  }

  if (!m_network->bind(m_listen_address)) {
    return false;
  }

  if (!m_network->listen()) {
    return false;
  }

  return true;
}

void Broker::spin() {
  using namespace std::chrono_literals;
  const size_t buffer_size = 64 * 1024;  // 64K is maximum UDP datagram size
  std::array<unsigned char, buffer_size> buffer{};

  for (;;) {
    std::string client;
    ssize_t bytes_received_signed = m_network->recvfrom(buffer.data(), buffer_size, &client);
    m_os->logger().debug() << "received " << bytes_received_signed << " bytes from " << client << "\n";
    if (bytes_received_signed < 0) {
      m_os->logger().error() << "recvfrom() failed. This should not happen! Continuing.\n";
      continue;
    }
    auto bytes_received = static_cast<size_t>(bytes_received_signed);
    if (bytes_received < bp_header_size) {
      m_os->logger().error() << "Received packet shorter than BpHeader from " << client
                             << ". This indicates a serious issue! Ignoring packet and continuing.\n";
      continue;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* bp_header = reinterpret_cast<BpHeader*>(buffer.data());
    if (!verify_and_update_client_counter(client, ntohl(bp_header->counter))) {
      m_os->logger().error() << "BpHeader::counter from " << client
                             << " is wrong. This indicates lost packets! Ignoring packet and continuing.\n";
      continue;
    }
    if (bp_header->type == BpType::Control) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      auto* bp_control_header = reinterpret_cast<BpControlHeader*>(&buffer[bp_header_size]);
      uint32_t json_end_index = bp_header_size + bp_control_header_size + ntohl(bp_control_header->size);
      if (json_end_index >= buffer_size) {
        m_os->logger().error() << "BpControlHeader::size from " << client
                               << " is wrong. This indicates corrupted packet data! Ignoring packet and continuing.\n";
        continue;
      }
      auto request = json::parse(&buffer[bp_header_size + bp_control_header_size],
                                 // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                                 &buffer[json_end_index]);
      if (!send_control_response(client, process_control_request(request))) {
        m_os->logger().error() << "Failed to send control response.\n";
      }
    } else if (bp_header->type == BpType::Data) {
      // TODO(ahb)
      m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
      break;
    }
  }
}

bool Broker::verify_and_update_client_counter(const std::string& p_client, BpCounter p_counter) {
  if (m_client_counters.find(p_client) == m_client_counters.end()) {  // new client
    m_client_counters[p_client] = p_counter;
  }
  return m_client_counters[p_client] == p_counter;
}

json Broker::process_control_request(const json& p_request) {
  json response{};
  m_os->logger().debug() << "Processing control request: " << p_request << "\n";
  if (p_request["node"]["action"] == "register_node") {
    BpNodeId node_id{};
    if (!register_node(p_request["node"]["params"]["node_name"], &node_id)) {
      response = R"(
        {
          "rpc_id": 0,
          "success": false,
          "error_code": 0,
          "error_message": "Failed to register node"
        }
      )"_json;
      response["rpc_id"] = p_request["rpc_id"];
      return response;
    }

    response = R"(
      {
        "rpc_id": 0,
        "success": true,
        "node_id": 0
        }
      )"_json;
    response["rpc_id"] = p_request["rpc_id"];
    response["node_id"] = node_id;
  } else {
    // TODO(ahb)
    m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
  }
  return response;
}

bool Broker::register_node(const std::string& p_node_name, BpNodeId* p_node_id) {
  auto node_info =
      std::find_if(m_nodes.begin(), m_nodes.end(), [&](const auto& ni) { return ni.node_name == p_node_name; });
  if (node_info == m_nodes.end()) {
    m_nodes.emplace_back(NodeInfo{p_node_name});
    *p_node_id = m_nodes.size() - 1;
    m_os->logger().debug() << "Registered node '" << p_node_name << "' with Node ID " << *p_node_id << ".\n";
    return true;
  }

  m_os->logger().warn() << "Already registered node '" << p_node_name << "' tried to register again.\n";
  return false;
}

bool Broker::send_control_response(const std::string& p_address, const json& p_response) {
  std::vector<unsigned char> buffer;
  bp_control_json_to_packet_buffer(p_response, m_client_counters[p_address], &buffer);
  m_os->logger().debug() << "send_control_response(" << p_response.dump() << ") " << buffer.size() << " bytes\n";
  return m_network->sendto(p_address, buffer.data(), buffer.size());
}

}  // namespace crps