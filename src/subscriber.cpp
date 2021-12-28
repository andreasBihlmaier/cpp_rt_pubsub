#include "crps/subscriber.h"

namespace crps {

Subscriber::Subscriber(std::string p_topic_name, TopicTypeId p_topic_type_id)
    : m_topic_name(std::move(p_topic_name)), m_topic_type_id(p_topic_type_id) {
  // TODO(ahb)
  (void)m_topic_type_id;  // TODO(ahb) prevent clang-diagnostic-unused-private-field -> remove
}

}  // namespace crps