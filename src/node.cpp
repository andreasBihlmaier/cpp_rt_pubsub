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
  m_subscribers.emplace_back(std::move(p_topic_name), std::move(p_type_name), p_message_size, std::move(p_callback),
                             p_callback_user_data, p_topic_priority);
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
  }

  if (m_topics.find(p_publisher->topic_id()) == m_topics.end()) {
    m_topics[p_publisher->topic_id()] = TopicInfo{};
  }
  m_topics[p_publisher->topic_id()].publishers.push_back(p_publisher);
  if (m_topics_by_prio.find(p_publisher->topic_priority()) == m_topics_by_prio.end()) {
    m_topics_by_prio[p_publisher->topic_priority()] = TopicInfo{};
  }
  m_topics_by_prio[p_publisher->topic_priority()].publishers.push_back(p_publisher);

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
  }

  if (m_topics.find(p_subscriber->topic_id()) == m_topics.end()) {
    m_topics[p_subscriber->topic_id()] = TopicInfo{};
  }
  m_topics[p_subscriber->topic_id()].subscribers.push_back(p_subscriber);
  if (m_topics_by_prio.find(p_subscriber->topic_priority()) == m_topics_by_prio.end()) {
    m_topics_by_prio[p_subscriber->topic_priority()] = TopicInfo{};
  }
  m_topics_by_prio[p_subscriber->topic_priority()].subscribers.push_back(p_subscriber);

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
  {
    int messages_processed = process_incoming_message();
    if (messages_processed < 0) {
      m_os->logger().error() << "process_incoming_message() failed. Continuing.\n";
    } else if (messages_processed > 0) {
      work_done = true;
    }
  }

  // process subscriber callbacks and outgoing publisher messages
  for (auto& topic_info : m_topics_by_prio) {  // iterated in priority order
    for (auto* subscriber : topic_info.second.subscribers) {
      if (subscriber->messages_queued() != 0) {
        CRPS_LOGGER_DEBUG(m_os, << "subscriber on topic '" << subscriber->topic_name() << "' with priority "
                                << static_cast<int>(topic_info.first) << " has " << subscriber->messages_queued()
                                << " incoming messages queued.\n");
        if (!subscriber->run_callback()) {
          m_os->logger().error() << "Run callback on subscriber of topic '" << subscriber->topic_name()
                                 << "' failed.\n";
          return SpinOnceResult::Error;
        }
      }
    }
    for (auto* publisher : topic_info.second.publishers) {
      if (publisher->messages_queued() != 0) {
        CRPS_LOGGER_DEBUG(m_os, << "publisher on topic '" << publisher->topic_name() << "' with priority "
                                << static_cast<int>(topic_info.first) << " has " << publisher->messages_queued()
                                << " outgoing messages queued.\n");
        if (!send_data(publisher->topic_id(), publisher->message_type_id(), publisher->get_message(),
                       publisher->packet_size())) {
          m_os->logger().error() << "Sending data failed.\n";
          return SpinOnceResult::Error;
        }
        publisher->pop_message();
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

  BpType bp_type;
  json control_json{};
  if (receive_message(true, &bp_type, nullptr, nullptr, nullptr, &control_json) <= 0) {
    m_os->logger().error() << "receive_message() failed.\n";
    return json{};
  }
  if (bp_type != BpType::Control) {
    // TODO(ahb)
    m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
    return json{};
  }
  return control_json;
}

bool Node::send_control(const json& p_request) {
  std::vector<unsigned char> buffer;
  bp_control_json_to_packet_buffer(p_request, next_bp_counter(), &buffer);
  CRPS_LOGGER_DEBUG(m_os, << "send_control(" << p_request.dump() << ") " << buffer.size() << " bytes\n");
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

ssize_t Node::receive_message(bool p_block, BpType* p_bp_type, BpHeader* p_bp_header,
                              BpControlHeader* p_bp_control_header, BpDataHeader* p_bp_data_header,
                              json* p_control_json) {
  ssize_t bytes_received_signed = m_network->recvfrom(m_buffer.data(), buffer_size, p_block, nullptr);
  if (bytes_received_signed < 0) {
    if (p_block || (errno != EAGAIN && errno != EWOULDBLOCK)) {
      m_os->logger().error() << "recvfrom() failed. This should not happen!\n";
      return -1;
    }
    return 0;
  }
  auto bytes_received{static_cast<size_t>(bytes_received_signed)};
  CRPS_LOGGER_DEBUG(m_os, << "received " << bytes_received_signed << " bytes\n");

  BpType bp_type;
  BpHeader bp_header{};
  BpControlHeader bp_control_header{};
  BpDataHeader bp_data_header{};
  json control_json{};
  {
    auto logger{m_os->logger()};
    bp_type = decode_packet_buffer(m_buffer.data(), bytes_received, &bp_header, &bp_control_header, &bp_data_header,
                                   &control_json, &logger);
  }

  // TODO(ahb) verify_and_update_broker_counter(bp_header.counter)

  if (bp_type == BpType::Invalid) {
    return -1;
  }

  if (p_bp_type != nullptr) {
    *p_bp_type = bp_type;
  }
  if (p_bp_header != nullptr) {
    *p_bp_header = bp_header;
  }
  if (p_bp_control_header != nullptr) {
    *p_bp_control_header = bp_control_header;
  }
  if (p_bp_data_header != nullptr) {
    *p_bp_data_header = bp_data_header;
  }
  if (p_control_json != nullptr) {
    *p_control_json = control_json;
  }
  return bytes_received_signed;
}

int Node::process_incoming_message() {
  BpType bp_type;
  BpHeader bp_header{};
  BpControlHeader bp_control_header{};
  BpDataHeader bp_data_header{};
  json control_json{};
  ssize_t bytes_received =
      receive_message(false, &bp_type, &bp_header, &bp_control_header, &bp_data_header, &control_json);
  if (bytes_received < 0) {
    m_os->logger().error() << "receive_message() failed.\n";
    return -1;
  }
  if (bytes_received == 0) {  // nothing to do
    return 0;
  }

  if (bp_type != BpType::Data) {
    // TODO(ahb)
    m_os->logger().error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
    return -1;
  }

  CRPS_LOGGER_DEBUG(m_os, << "Received message for topic ID " << bp_data_header.topic_id << ".\n");
  if (m_topics.find(bp_data_header.topic_id) == m_topics.end()) {
    m_os->logger().error() << "Received message for unknown topic ID " << bp_data_header.topic_id << ".\n";
    return -1;
  }
  auto& topic_info = m_topics[bp_data_header.topic_id];

  if (topic_info.subscribers.empty()) {
    m_os->logger().error() << "Received message for topic ID " << bp_data_header.topic_id
                           << " but it has no subscribers.\n";
    return -1;
  }

  if (bp_data_header.message_type_id != (*topic_info.subscribers.begin())->message_type_id()) {
    m_os->logger().error() << "Received message for topic ID " << bp_data_header.topic_id
                           << " with wrong message type ID (got: " << bp_data_header.message_type_id
                           << "; expected: " << (*topic_info.subscribers.begin())->message_type_id() << ").\n";
  }
  for (auto* subscriber : topic_info.subscribers) {
    subscriber->queue_message(m_buffer.data() + bp_header_size + bp_data_header_size);
  }

  return 1;
}

}  // namespace crps