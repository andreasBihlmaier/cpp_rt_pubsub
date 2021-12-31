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
  explicit Subscriber(std::string p_topic_name, std::string p_type_name, MessageSize p_message_size,
                      SubscriberCallback p_callback, void* p_callback_user_data = nullptr);

 private:
  std::string m_topic_name;
  std::string m_type_name;
  MessageSize m_message_size;
  SubscriberCallback m_callback;
  void* m_callback_user_data;
  TopicId m_topic_id = 0;
  MessageTypeId m_type_id = 0;
};

}  // namespace crps

#endif  // CRPS_SUBSCRIBER_H