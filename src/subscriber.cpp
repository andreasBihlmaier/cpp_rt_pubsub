#include "crps/subscriber.h"

#include "crps/defaults.h"

namespace crps {

Subscriber::Subscriber(std::string p_topic_name, std::string p_message_type_name, MessageSize p_message_size,
                       SubscriberCallback p_callback, void* p_callback_user_data, TopicPriority p_topic_priority)
    : m_topic_name(std::move(p_topic_name)),
      m_message_type_name(std::move(p_message_type_name)),
      m_message_size(p_message_size),
      m_callback(std::move(p_callback)),
      m_callback_user_data(p_callback_user_data),
      m_topic_priority(p_topic_priority),
      m_message_queue(p_message_size, message_queue_capacity) {
}

bool Subscriber::queue_message(const void* data) {
  return m_message_queue.push(data);
}

bool Subscriber::run_callback() {
  if (m_message_queue.empty()) {
    return false;
  }
  m_callback(m_message_queue.get(), m_message_size, m_callback_user_data);
  return m_message_queue.pop();
}

}  // namespace crps