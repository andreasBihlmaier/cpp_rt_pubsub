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

  for (auto& publisher : m_publishers) {
    if (!register_publisher(&publisher)) {
      return false;
    }
  }

  for (auto& subscriber : m_subscribers) {
    if (!register_subscriber(&subscriber)) {
      return false;
    }
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
      "scope": "node",
      "node": {
        "action": "register",
        "params": {
        }
      }
    }
  )"_json;
  request["rpc_id"] = rpc_id();
  request["node"]["params"]["name"] = m_name;
  auto result = broker_rpc_blocking(request);
  if (result.empty() || !result["success"]) {
    return false;
  }
  m_node_id = result["node_id"];
  return true;
}

bool Node::register_publisher(Publisher* p_publisher) {
  {  // register message type
    auto request = R"(
    {
      "scope": "message",
      "message": {
        "action": "register",
        "params": {
        }
      }
    }
  )"_json;
    request["rpc_id"] = rpc_id();
    request["message"]["params"]["name"] = p_publisher->message_type_name();
    request["message"]["params"]["size"] = p_publisher->message_size();
    auto result = broker_rpc_blocking(request);
    if (result.empty() || !result["success"]) {
      return false;
    }
    p_publisher->set_message_type_id(result["message_type_id"]);
  }

  {  // register topic
    auto request = R"(
    {
      "scope": "topic",
      "topic": {
        "action": "register",
        "params": {
        }
      }
    }
  )"_json;
    request["rpc_id"] = rpc_id();
    request["topic"]["params"]["name"] = p_publisher->topic_name();
    request["topic"]["params"]["message_type_id"] = p_publisher->message_type_id();
    request["topic"]["params"]["priority"] = p_publisher->topic_priority();
    auto result = broker_rpc_blocking(request);
    if (result.empty() || !result["success"]) {
      return false;
    }
    // TODO(ahb) use result["..."]
    (void)result;
  }

  {  // add publisher
    auto request = R"(
    {
      "scope": "topic",
      "topic": {
        "action": "add_publisher",
        "params": {
        }
      }
    }
  )"_json;
    request["rpc_id"] = rpc_id();
    request["topic"]["params"]["topic_id"] = p_publisher->topic_id();
    request["topic"]["params"]["message_type_id"] = p_publisher->message_type_id();
    auto result = broker_rpc_blocking(request);
    if (result.empty() || !result["success"]) {
      return false;
    }
    // TODO(ahb) use result["..."]
    (void)result;
  }

  return true;
}

bool Node::register_subscriber(Subscriber* p_subscriber) {
  m_os->logger().debug() << "Registering subscriber " << p_subscriber->topic_name() << "\n";
  return true;  // TODO(ahb)
}

json Node::broker_rpc_blocking(const json& p_request) {
  if (!send_control(p_request)) {
    m_os->logger().error() << "send_control(" << p_request << ") failed.\n";
    return json{};
  }

  // TODO(ahb) refactor START vvv
  const size_t buffer_size = 64 * 1024;  // 64K is maximum UDP datagram size
  std::array<unsigned char, buffer_size> buffer{};
  ssize_t bytes_received_signed = m_network->recvfrom(buffer.data(), buffer_size, nullptr);
  m_os->logger().debug() << "received " << bytes_received_signed << " bytes\n";
  if (bytes_received_signed < 0) {
    m_os->logger().error() << "recvfrom() failed. This should not happen!\n";
    return json{};
  }
  auto bytes_received{static_cast<size_t>(bytes_received_signed)};

  BpType bp_type;
  BpHeader bp_header{};
  BpControlHeader bp_control_header{};
  BpDataHeader bp_data_header{};
  json control_json{};
  {
    auto logger{m_os->logger()};
    bp_type = decode_packet_buffer(buffer.data(), bytes_received, &bp_header, &bp_control_header, &bp_data_header,
                                   &control_json, &logger);
  }

  // TODO(ahb) verify_and_update_broker_counter(bp_header.counter)

  if (bp_type == BpType::Invalid) {
    m_os->logger().error() << "decode_packet_buffer() failed.\n";
  } else if (bp_type == BpType::Control) {
    m_os->logger().debug() << "received control response: " << control_json << "\n";
    return control_json;
  } else if (bp_type == BpType::Data) {
    // TODO(ahb)
    m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
    return json{};
  }
  // TODO(ahb) refactor END ^^^

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

uint32_t Node::rpc_id() const {
  return m_bp_counter;  // Use m_bp_counter as an unique-per-node-per-ongoing-transaction number
}

}  // namespace crps