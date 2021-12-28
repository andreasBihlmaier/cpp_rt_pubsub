#ifndef CRPS_SUBSCRIBER_H
#define CRPS_SUBSCRIBER_H

#include <string>

#include "crps/message.h"
#include "crps/topic.h"

namespace crps {

using SubscriberCallback = void (*)(void* p_message, MessageSize p_message_size, void* p_user_data);

class Subscriber {
 private:
  std::string m_topic_name;
  MessageTypeId m_type_id;
  MessageSize m_message_size;
  SubscriberCallback m_callback;
  void* m_callback_user_data;

 public:
  explicit Subscriber(std::string p_topic_name, MessageTypeId p_type_id, MessageSize p_message_size,
                      SubscriberCallback p_callback, void* p_callback_user_data = nullptr);
  bool connect();
};

}  // namespace crps

#endif  // CRPS_SUBSCRIBER_H