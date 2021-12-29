#include "crps/node.h"

namespace crps {

Node::Node(std::string p_name, std::string p_broker_address, Network* p_network, Network::Protocol p_protocol,
           int16_t p_port)
    : m_name(std::move(p_name)),
      m_broker_address(std::move(p_broker_address)),
      m_network(p_network),
      m_protocol(p_protocol),
      m_port(p_port) {
  // TODO(ahb) prevent clang-diagnostic-unused-private-field -> remove
  (void)m_name;
  (void)m_network;
}

bool Node::connect() {
  if (!m_network->socket(m_protocol)) {
    return false;
  }

  if (!m_network->connect(m_broker_address, m_port)) {
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

}  // namespace crps