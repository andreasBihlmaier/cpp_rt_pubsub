#ifndef CRPS_PUBLISHER_H
#define CRPS_PUBLISHER_H

#include <string>

#include "crps/message.h"
#include "crps/topic.h"

namespace crps {

class Publisher {
 private:
  std::string m_topic_name;
  MessageTypeId m_type_id;
  MessageSize m_message_size;
  TopicPriority m_topic_priority;

 public:
  explicit Publisher(std::string p_topic_name, MessageTypeId p_type_id, MessageSize p_message_size,
                     TopicPriority p_topic_priority);
  bool publish(void* data);
};

}  // namespace crps

#endif  // CRPS_PUBLISHER_H