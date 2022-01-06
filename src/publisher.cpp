#include "crps/publisher.h"

#include "crps/defaults.h"

namespace crps {

Publisher::Publisher(std::string p_topic_name, std::string p_message_type_name, MessageSize p_message_size,
                     TopicPriority p_topic_priority, size_t p_packet_header_size)
    : m_topic_name(std::move(p_topic_name)),
      m_message_type_name(std::move(p_message_type_name)),
      m_message_size(p_message_size),
      m_topic_priority(p_topic_priority),
      m_packet_header_size(p_packet_header_size),
      m_message_queue(p_packet_header_size + p_message_size, message_queue_capacity) {
}

bool Publisher::publish(const void* data) {
  return m_message_queue.push(data, nullptr, m_packet_header_size);
}

}  // namespace crps