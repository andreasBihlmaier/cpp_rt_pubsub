#ifndef CRPS_PUBLISHER_H
#define CRPS_PUBLISHER_H

#include <string>

#include "crps/message.h"
#include "crps/message_queue.h"
#include "crps/topic.h"

namespace crps {

class Publisher {
 public:
  explicit Publisher(std::string p_topic_name, std::string p_message_type_name, MessageSize p_message_size,
                     TopicPriority p_topic_priority, size_t p_packet_header_size);
  bool publish(const void* data);
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
  [[nodiscard]] size_t messages_queued() {
    return m_message_queue.size();
  }
  [[nodiscard]] void* get_message() const {
    return m_message_queue.get();
  }
  bool pop_message() {
    return m_message_queue.pop();
  }

 private:
  std::string m_topic_name;
  std::string m_message_type_name;
  MessageSize m_message_size;
  TopicPriority m_topic_priority;
  size_t m_packet_header_size;
  TopicId m_topic_id = 0;
  MessageTypeId m_message_type_id = 0;
  MessageQueue m_message_queue;
};

}  // namespace crps

#endif  // CRPS_PUBLISHER_H