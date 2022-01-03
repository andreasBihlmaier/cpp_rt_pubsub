#include "crps/subscriber.h"

namespace crps {

Subscriber::Subscriber(std::string p_topic_name, std::string p_message_type_name, MessageSize p_message_size,
                       SubscriberCallback p_callback, void* p_callback_user_data)
    : m_topic_name(std::move(p_topic_name)),
      m_message_type_name(std::move(p_message_type_name)),
      m_message_size(p_message_size),
      m_callback(std::move(p_callback)),
      m_callback_user_data(p_callback_user_data) {
  // TODO(ahb) prevent clang-diagnostic-unused-private-field -> remove
  (void)m_topic_id;
  (void)m_message_type_id;
  (void)m_message_size;
  (void)m_callback;
  (void)m_callback_user_data;
}

}  // namespace crps