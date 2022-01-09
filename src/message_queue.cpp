#include "crps/message_queue.h"

namespace crps {

MessageQueue::MessageQueue(MessageSize p_message_size, size_t p_queue_capacity)
    : m_message_size(p_message_size), m_queue_capacity(p_queue_capacity) {
  for (size_t i = 0; i < m_queue_capacity; ++i) {
    m_buffers.emplace_back(m_message_size, 0);
    m_free_buffers.push_back(&m_buffers.back());
  }
}

}  // namespace crps