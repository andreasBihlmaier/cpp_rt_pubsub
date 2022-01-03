#ifndef CRPS_TOPIC_H
#define CRPS_TOPIC_H

#include <cstdint>
#include <string>

namespace crps {

using TopicId = uint32_t;
using TopicPriority = int8_t;

const TopicPriority dontcare_topic_priority = -101;
const TopicPriority lowest_topic_priority = -100;
const TopicPriority highest_topic_priority = 100;

}  // namespace crps

#endif  // CRPS_TOPIC_H