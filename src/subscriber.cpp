#include "crps/subscriber.h"

#include "crps/defaults.h"

namespace crps {

Subscriber::Subscriber(std::string p_topic_name, std::string p_message_type_name, MessageSize p_message_size,
                       size_t p_packet_header_size, SubscriberCallback p_callback, void* p_callback_user_data,
                       TopicPriority p_topic_priority)
    : m_topic_name(std::move(p_topic_name)),
      m_message_type_name(std::move(p_message_type_name)),
      m_message_size(p_message_size),
      m_packet_header_size(p_packet_header_size),
      m_callback(std::move(p_callback)),
      m_callback_user_data(p_callback_user_data),
      m_topic_priority(p_topic_priority),
      m_message_queue(p_packet_header_size + p_message_size, message_queue_capacity) {
  // TODO(ahb) prevent clang-diagnostic-unused-private-field -> remove
  (void)m_callback_user_data;
}

}  // namespace crps