#include "crps/publisher.h"

namespace crps {

Publisher::Publisher(std::string p_topic_name, MessageTypeId p_type_id, MessageSize p_message_size,
                     TopicPriority p_topic_priority)
    : m_topic_name(std::move(p_topic_name)),
      m_type_id(p_type_id),
      m_message_size(p_message_size),
      m_topic_priority(p_topic_priority) {
  // TODO(ahb) prevent clang-diagnostic-unused-private-field -> remove
  (void)m_type_id;
  (void)m_message_size;
  (void)m_topic_priority;
}

bool Publisher::connect() {
  (void)m_type_id;  // TODO(ahb) prevent readability-convert-member-functions-to-static -> remove

  // TODO(ahb)

  return true;
}

}  // namespace crps