#ifndef CRPS_MESSAGE_QUEUE_H
#define CRPS_MESSAGE_QUEUE_H

#include <cstring>
#include <list>
#include <queue>

#include "crps/message.h"

namespace crps {

class MessageQueue {
 public:
  explicit MessageQueue(MessageSize p_message_size, size_t p_queue_capacity);
  [[nodiscard]] bool empty() const {
    return m_queue.empty();
  }
  [[nodiscard]] bool full() const {
    return m_free_buffers.empty();
  }
  [[nodiscard]] size_t size() const {
    return m_queue.size();
  }
  [[nodiscard]] void* get() const {
    if (m_queue.empty()) {
      return nullptr;
    }
    return m_queue.front()->data();
  }
  bool pop() {
    if (m_queue.empty()) {
      return false;
    }
    m_free_buffers.push_front(m_queue.front());
    m_queue.pop();
    return true;
  }
  bool push(const void* p_data, const void* p_header_data = nullptr, size_t p_header_size = 0) {
    if (m_free_buffers.empty()) {
      return false;
    }
    auto* buffer = m_free_buffers.front();
    m_free_buffers.pop_front();
    if (p_header_data != nullptr) {
      std::memcpy(buffer->data(), p_header_data, p_header_size);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::memcpy(buffer->data() + p_header_size, p_data, m_message_size - p_header_size);
    m_queue.push(buffer);
    return true;
  }

 private:
  using MessageBuffer = std::vector<unsigned char>;
  MessageSize m_message_size;
  size_t m_queue_capacity;
  std::list<MessageBuffer> m_buffers;
  std::list<MessageBuffer*> m_free_buffers;
  std::queue<MessageBuffer*> m_queue;
};

}  // namespace crps

#endif  // CRPS_MESSAGE_QUEUE_H