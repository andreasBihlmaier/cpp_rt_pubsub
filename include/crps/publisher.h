#ifndef CRPS_PUBLISHER_H
#define CRPS_PUBLISHER_H

#include <string>

#include "crps/message.h"
#include "crps/topic.h"

namespace crps {

class Publisher {
 public:
  explicit Publisher(std::string p_topic_name, std::string p_type_name, MessageSize p_message_size,
                     TopicPriority p_topic_priority);
  bool publish(void* data);

 private:
  std::string m_topic_name;
  std::string m_type_name;
  MessageSize m_message_size;
  TopicPriority m_topic_priority;
  TopicId m_topic_id = 0;
  MessageTypeId m_type_id = 0;
};

}  // namespace crps

#endif  // CRPS_PUBLISHER_H