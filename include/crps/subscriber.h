#ifndef CRPS_SUBSCRIBER_H
#define CRPS_SUBSCRIBER_H

#include <string>

#include "crps/topic.h"

namespace crps {

class Subscriber {
 private:
  std::string m_topic_name;
  TopicTypeId m_topic_type_id;

 public:
  explicit Subscriber(std::string p_topic_name, TopicTypeId p_topic_type_id);
};

}  // namespace crps

#endif  // CRPS_SUBSCRIBER_H