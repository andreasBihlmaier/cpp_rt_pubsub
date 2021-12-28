#include "crps/node.h"

namespace crps {

Node::Node(std::string p_name, Network* p_network) : m_name(std::move(p_name)), m_network(p_network) {
  // TODO(ahb) prevent clang-diagnostic-unused-private-field -> remove
  (void)m_name;
  (void)m_network;
}

bool Node::connect() {
  (void)m_name;  // TODO(ahb) prevent readability-convert-member-functions-to-static -> remove

  // TODO(ahb)

  return true;
}

Publisher* Node::create_publisher(std::string p_topic_name, MessageTypeId p_type_id, MessageSize p_message_size,
                                  TopicPriority p_topic_priority) {
  m_publishers.emplace_back(std::move(p_topic_name), p_type_id, p_message_size, p_topic_priority);
  return &*m_publishers.end();
}

Subscriber* Node::create_subscriber(std::string p_topic_name, MessageTypeId p_type_id, MessageSize p_message_size,
                                    SubscriberCallback p_callback, void* p_callback_user_data) {
  m_subscribers.emplace_back(std::move(p_topic_name), p_type_id, p_message_size, p_callback, p_callback_user_data);
  return &*m_subscribers.end();
}

// TODO(ahb)
// Node iterates [1] through m_publishers and m_subscribers and performs actions (send / receive+callback) based on
// topic priority.
// [1] single thread with priorities as data or multiple threads with different thread priorities?

}  // namespace crps