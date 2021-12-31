#include "crps/node.h"

#include <cstring>

namespace crps {

Node::Node(std::string p_name, std::string p_broker_address, OS* p_os, Network* p_network, Network::Protocol p_protocol,
           int16_t p_port)
    : m_name(std::move(p_name)),
      m_broker_address(std::move(p_broker_address)),
      m_os(p_os),
      m_network(p_network),
      m_protocol(p_protocol),
      m_port(p_port) {
  // TODO(ahb) prevent clang-diagnostic-unused-private-field -> remove
  (void)m_node_id;
}

bool Node::connect() {
  if (!m_network->socket(m_protocol)) {
    return false;
  }

  if (!m_network->connect(m_broker_address, m_port)) {
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
  auto cmd = R"(
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
  cmd["rpc_id"] = m_bp_counter;  // Use m_bp_counter as an unique-per-node-per-ongoing-transaction number
  cmd["node"]["params"]["node_name"] = m_name;
  auto result = broker_rpc_blocking(cmd);
  if (result.empty()) {
    return false;
  }
  // TODO(ahb)
  (void)result;
  return true;
}

json Node::broker_rpc_blocking(const json& cmd) {
  if (!bp_send_control(cmd.dump())) {
    m_os->logger().error() << "bp_send_control(" << cmd << ") failed.\n";
    return json{};
  }
  // TODO(ahb) get response
  return json{};  // TODO(ahb)
}

bool Node::bp_send_control(const std::string& cmd) {
  std::vector<unsigned char> buffer(bp_header_size + bp_control_header_size + cmd.size());
  m_os->logger().debug() << "bp_send_control(" << cmd << ") " << buffer.size() << " bytes\n";
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* bp_header = reinterpret_cast<BpHeader*>(buffer.data());
  bp_header->type = BpType::Control;
  bp_header->counter = next_bp_counter();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* bp_control_header = reinterpret_cast<BpControlHeader*>(&buffer[bp_header_size]);
  bp_control_header->size = cmd.size();
  std::memcpy(&buffer[bp_header_size + bp_control_header_size], &cmd[0], cmd.size());
  return m_network->sendto(m_broker_address, buffer.data(), buffer.size());
}

BpCounterType Node::next_bp_counter() {
  m_bp_counter += 1;
  return m_bp_counter;
}

}  // namespace crps