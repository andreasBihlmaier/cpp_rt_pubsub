#include "crps/node.h"

#include <cstring>

namespace crps {

Node::Node(std::string p_name, std::string p_broker_host, OS* p_os, Network* p_network, Network::Protocol p_protocol,
           int16_t p_port)
    : m_name(std::move(p_name)),
      m_broker_address(std::move(p_broker_host) + ":" + std::to_string(p_port)),
      m_os(p_os),
      m_network(p_network),
      m_protocol(p_protocol) {
}

bool Node::connect() {
  if (!m_network->socket(m_protocol)) {
    return false;
  }

  if (!m_network->connect(m_broker_address)) {
    return false;
  }

  if (!register_node()) {
    return false;
  }

  return true;
}

Publisher* Node::create_publisher(std::string p_topic_name, std::string p_type_name, MessageSize p_message_size,
                                  TopicPriority p_topic_priority) {
  m_publishers.emplace_back(std::move(p_topic_name), std::move(p_type_name), p_message_size, p_topic_priority);
  return &*m_publishers.end();
}

Subscriber* Node::create_subscriber(std::string p_topic_name, std::string p_type_name, MessageSize p_message_size,
                                    SubscriberCallback p_callback, void* p_callback_user_data) {
  m_subscribers.emplace_back(std::move(p_topic_name), std::move(p_type_name), p_message_size, std::move(p_callback),
                             p_callback_user_data);
  return &*m_subscribers.end();
}

// TODO(ahb)
// Node iterates [1] through m_publishers and m_subscribers and performs actions (send / receive+callback) based on
// topic priority.
// [1] single thread with priorities as data or multiple threads with different thread priorities?

bool Node::register_node() {
  auto request = R"(
    {
      "rpc_id": 0,
      "scope": "node",
      "node": {
        "action": "register_node",
        "params": {
          "node_name": ""
        }
      }
    }
  )"_json;
  request["rpc_id"] = m_bp_counter;  // Use m_bp_counter as an unique-per-node-per-ongoing-transaction number
  request["node"]["params"]["node_name"] = m_name;
  auto result = broker_rpc_blocking(request);
  if (result.empty()) {
    return false;
  }
  // TODO(ahb)
  (void)result;
  (void)m_node_id;
  return true;
}

json Node::broker_rpc_blocking(const json& p_request) {
  if (!send_control(p_request)) {
    m_os->logger().error() << "send_control(" << p_request << ") failed.\n";
    return json{};
  }

  // TODO(ahb) refactor vvv
  const size_t buffer_size = 64 * 1024;  // 64K is maximum UDP datagram size
  std::array<unsigned char, buffer_size> buffer{};
  ssize_t bytes_received_signed = m_network->recvfrom(buffer.data(), buffer_size, nullptr);
  m_os->logger().debug() << "received " << bytes_received_signed << " bytes\n";
  // TODO(ahb) get response

  return json{};  // TODO(ahb)
}

bool Node::send_control(const json& p_request) {
  std::vector<unsigned char> buffer;
  bp_control_json_to_packet_buffer(p_request, next_bp_counter(), &buffer);
  m_os->logger().debug() << "send_control(" << p_request.dump() << ") " << buffer.size() << " bytes\n";
  return m_network->sendto(m_broker_address, buffer.data(), buffer.size());
}

BpCounter Node::next_bp_counter() {
  m_bp_counter += 1;
  return m_bp_counter;
}

}  // namespace crps