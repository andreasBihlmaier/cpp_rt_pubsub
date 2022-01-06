#ifndef CRPS_SUBSCRIBER_H
#define CRPS_SUBSCRIBER_H

#include <functional>
#include <string>

#include "crps/message.h"
#include "crps/message_queue.h"
#include "crps/topic.h"

namespace crps {

using SubscriberCallback = std::function<void(void* p_message, MessageSize p_message_size, void* p_user_data)>;

class Subscriber {
 public:
  explicit Subscriber(std::string p_topic_name, std::string p_message_type_name, MessageSize p_message_size,
                      size_t p_packet_header_size, SubscriberCallback p_callback, void* p_callback_user_data = nullptr,
                      TopicPriority p_topic_priority = dontcare_topic_priority);
  [[nodiscard]] std::string topic_name() const {
    return m_topic_name;
  }
  [[nodiscard]] std::string message_type_name() const {
    return m_message_type_name;
  }
  [[nodiscard]] MessageSize message_size() const {
    return m_message_size;
  }
  [[nodiscard]] size_t packet_size() const {
    return m_packet_header_size + m_message_size;
  }
  [[nodiscard]] TopicPriority topic_priority() const {
    return m_topic_priority;
  }
  [[nodiscard]] TopicId topic_id() const {
    return m_topic_id;
  }
  [[nodiscard]] MessageTypeId message_type_id() const {
    return m_message_type_id;
  }
  void set_message_type_id(MessageTypeId p_message_type_id) {
    m_message_type_id = p_message_type_id;
  }
  void set_topic_id(TopicId p_topic_id) {
    m_topic_id = p_topic_id;
  }
  void set_topic_priority(TopicPriority p_topic_priority) {
    m_topic_priority = p_topic_priority;
  }
  [[nodiscard]] size_t messages_queued() {
    return m_message_queue.size();
  }

 private:
  std::string m_topic_name;
  std::string m_message_type_name;
  MessageSize m_message_size;
  size_t m_packet_header_size;
  SubscriberCallback m_callback;
  void* m_callback_user_data;
  TopicPriority m_topic_priority;
  TopicId m_topic_id = 0;
  MessageTypeId m_message_type_id = 0;
  MessageQueue m_message_queue;
};

}  // namespace crps

#endif  // CRPS_SUBSCRIBER_H