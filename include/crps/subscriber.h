#ifndef CRPS_SUBSCRIBER_H
#define CRPS_SUBSCRIBER_H

#include <functional>
#include <string>

#include "crps/message.h"
#include "crps/topic.h"

namespace crps {

using SubscriberCallback = std::function<void(void* p_message, MessageSize p_message_size, void* p_user_data)>;

class Subscriber {
 public:
  explicit Subscriber(std::string p_topic_name, std::string p_message_type_name, MessageSize p_message_size,
                      SubscriberCallback p_callback, void* p_callback_user_data = nullptr);
  [[nodiscard]] std::string topic_name() const {
    return m_topic_name;
  }
  [[nodiscard]] std::string message_type_name() const {
    return m_message_type_name;
  }
  [[nodiscard]] MessageSize message_size() const {
    return m_message_size;
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

 private:
  std::string m_topic_name;
  std::string m_message_type_name;
  MessageSize m_message_size;
  SubscriberCallback m_callback;
  void* m_callback_user_data;
  TopicId m_topic_id = 0;
  MessageTypeId m_message_type_id = 0;
};

}  // namespace crps

#endif  // CRPS_SUBSCRIBER_H