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

  for (unsigned i = 0; i < m_publishers.size(); ++i) {
    auto& publisher = m_publishers[i];
    if (!register_publisher(&publisher)) {
      return false;
    }
    if (m_topics.find(publisher.topic_priority()) == m_topics.end()) {
      m_topics[publisher.topic_priority()] = TopicInfo{};
    }
    m_topics[publisher.topic_priority()].publishers.push_back(i);
  }

  for (unsigned i = 0; i < m_subscribers.size(); ++i) {
    auto& subscriber = m_subscribers[i];
    if (!register_subscriber(&subscriber)) {
      return false;
    }
    if (m_topics.find(subscriber.topic_priority()) == m_topics.end()) {
      m_topics[subscriber.topic_priority()] = TopicInfo{};
    }
    m_topics[subscriber.topic_priority()].subscribers.push_back(i);
  }

  m_os->logger().info() << "Node '" << m_name << "' connected with ID " << m_node_id << ".\n";
  return true;
}

Publisher* Node::create_publisher(std::string p_topic_name, std::string p_type_name, MessageSize p_message_size,
                                  TopicPriority p_topic_priority) {
  m_publishers.emplace_back(std::move(p_topic_name), std::move(p_type_name), p_message_size, p_topic_priority,
                            bp_header_size + bp_data_header_size);
  return &m_publishers.back();
}

Subscriber* Node::create_subscriber(std::string p_topic_name, std::string p_type_name, MessageSize p_message_size,
                                    SubscriberCallback p_callback, void* p_callback_user_data,
                                    TopicPriority p_topic_priority) {
  m_subscribers.emplace_back(std::move(p_topic_name), std::move(p_type_name), p_message_size,
                             bp_header_size + bp_data_header_size, std::move(p_callback), p_callback_user_data,
                             p_topic_priority);
  return &m_subscribers.back();
}

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

bool Node::register_message_type(const std::string& p_message_type_name, MessageSize p_message_size,
                                 MessageTypeId* p_message_type_id) {
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
  request["message"]["params"]["name"] = p_message_type_name;
  request["message"]["params"]["size"] = p_message_size;
  auto result = broker_rpc_blocking(request);
  if (result.empty() || !result["success"]) {
    return false;
  }
  *p_message_type_id = result["message_type_id"];
  return true;
}

bool Node::register_topic(const std::string& p_topic_name, MessageTypeId p_message_type_id,
                          TopicPriority p_topic_priority, TopicId* p_topic_id, TopicPriority* p_actual_topic_priority) {
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
  request["topic"]["params"]["name"] = p_topic_name;
  request["topic"]["params"]["message_type_id"] = p_message_type_id;
  request["topic"]["params"]["priority"] = p_topic_priority;
  auto result = broker_rpc_blocking(request);
  if (result.empty() || !result["success"]) {
    return false;
  }
  *p_topic_id = result["topic_id"];
  if (p_actual_topic_priority != nullptr) {
    *p_actual_topic_priority = result["topic_priority"];
  }
  return true;
}

bool Node::register_publisher(Publisher* p_publisher) {
  {  // register message type
    MessageTypeId message_type_id{};
    if (!register_message_type(p_publisher->message_type_name(), p_publisher->message_size(), &message_type_id)) {
      return false;
    }
    p_publisher->set_message_type_id(message_type_id);
  }

  {  // register topic
    TopicId topic_id{};
    if (!register_topic(p_publisher->topic_name(), p_publisher->message_type_id(), p_publisher->topic_priority(),
                        &topic_id)) {
      return false;
    }
    p_publisher->set_topic_id(topic_id);
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
    request["topic"]["params"]["node_id"] = m_node_id;
    request["topic"]["params"]["topic_id"] = p_publisher->topic_id();
    request["topic"]["params"]["message_type_id"] = p_publisher->message_type_id();
    auto result = broker_rpc_blocking(request);
    if (result.empty() || !result["success"]) {
      return false;
    }
    (void)result;
  }

  return true;
}

bool Node::register_subscriber(Subscriber* p_subscriber) {
  {  // register message type
    MessageTypeId message_type_id{};
    if (!register_message_type(p_subscriber->message_type_name(), p_subscriber->message_size(), &message_type_id)) {
      return false;
    }
    p_subscriber->set_message_type_id(message_type_id);
  }

  {  // register topic
    TopicId topic_id{};
    TopicPriority actual_topic_priority{};
    if (!register_topic(p_subscriber->topic_name(), p_subscriber->message_type_id(), p_subscriber->topic_priority(),
                        &topic_id, &actual_topic_priority)) {
      return false;
    }
    p_subscriber->set_topic_id(topic_id);
    p_subscriber->set_topic_priority(actual_topic_priority);
  }

  {  // add subscriber
    auto request = R"(
    {
      "scope": "topic",
      "topic": {
        "action": "add_subscriber",
        "params": {
        }
      }
    }
  )"_json;
    request["rpc_id"] = rpc_id();
    request["topic"]["params"]["node_id"] = m_node_id;
    request["topic"]["params"]["topic_id"] = p_subscriber->topic_id();
    request["topic"]["params"]["message_type_id"] = p_subscriber->message_type_id();
    auto result = broker_rpc_blocking(request);
    if (result.empty() || !result["success"]) {
      return false;
    }
    (void)result;
  }

  return true;
}

bool Node::spin() {
  SpinOnceResult result{};
  do {
    result = spin_once();
    // TODO(ahb)
    // SpinOnceResult::NoWork leads to busy waiting. Implement something better, e.g. using std::condition_variable.
  } while (result == SpinOnceResult::Success || result == SpinOnceResult::NoWork);
  return false;
}

bool Node::spin_while_work() {
  SpinOnceResult result{};
  do {
    result = spin_once();
  } while (result == SpinOnceResult::Success);
  return (result == SpinOnceResult::NoWork);
}

Node::SpinOnceResult Node::spin_once() {
  bool work_done{false};
  // process incoming broker messages
  // TODO(ahb)
  // non-blocking!

  // process subscriber callbacks and outgoing publisher messages
  for (auto& topic_info : m_topics) {  // iterated in priority order
    for (auto subscriber_index : topic_info.second.subscribers) {
      auto& subscriber = m_subscribers[subscriber_index];
      if (subscriber.messages_queued() != 0) {
        m_os->logger().debug() << "subscriber " << subscriber_index << " on topic '" << subscriber.topic_name()
                               << "' with priority " << static_cast<int>(topic_info.first) << " has "
                               << subscriber.messages_queued() << " incoming messages queued\n";
        // TODO(ahb)
        m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
        return SpinOnceResult::Error;
      }
    }
    for (auto publisher_index : topic_info.second.publishers) {
      auto& publisher = m_publishers[publisher_index];
      if (publisher.messages_queued() != 0) {
        m_os->logger().debug() << "publisher " << publisher_index << " on topic '" << publisher.topic_name()
                               << "' with priority " << static_cast<int>(topic_info.first) << " has "
                               << publisher.messages_queued() << " outgoing messages queued\n";
        if (!send_data(publisher.topic_id(), publisher.message_type_id(), publisher.get_message(),
                       publisher.packet_size())) {
          m_os->logger().error() << "Sending data failed.\n";
          return SpinOnceResult::Error;
        }
        publisher.pop_message();
        work_done = true;
      }
    }
  }
  if (!work_done) {
    return SpinOnceResult::NoWork;
  }
  return SpinOnceResult::Success;
}

json Node::broker_rpc_blocking(const json& p_request) {
  if (!send_control(p_request)) {
    m_os->logger().error() << "send_control(" << p_request << ") failed.\n";
    return json{};
  }

  // TODO(ahb) refactor START vvv
  const size_t buffer_size = 64 * 1024;  // 64K is maximum UDP datagram size
  std::array<unsigned char, buffer_size> buffer{};
  ssize_t bytes_received_signed = m_network->recvfrom(buffer.data(), buffer_size, true, nullptr);
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

bool Node::send_data(TopicId p_topic_id, MessageTypeId p_message_type_id, void* p_buffer, size_t p_buffer_size) {
  bp_fill_data_header(next_bp_counter(), p_topic_id, p_message_type_id,
                      p_buffer_size - (bp_header_size + bp_data_header_size), p_buffer);
  return m_network->sendto(m_broker_address, p_buffer, p_buffer_size);
}

BpCounter Node::next_bp_counter() {
  m_bp_counter += 1;
  return m_bp_counter;
}

uint32_t Node::rpc_id() const {
  return m_bp_counter;  // Use m_bp_counter as an unique-per-node-per-ongoing-transaction number
}

}  // namespace crps