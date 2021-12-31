#ifndef CRPS_BROKER_PROTOCOL_H
#define CRPS_BROKER_PROTOCOL_H

#include <cstdint>

#include "crps/message.h"
#include "crps/topic.h"

namespace crps {

// bp = broker protocol

using BpCounterType = uint32_t;

enum class BpType : uint8_t {
  Invalid = 0,
  Control = 1,
  Data = 2,
};

struct BpHeader {
  BpType type;
  BpCounterType counter;
} __attribute__((packed));
const size_t bp_header_size = sizeof(BpHeader);

struct BpControlHeader {
  uint32_t size;
} __attribute__((packed));
const size_t bp_control_header_size = sizeof(BpControlHeader);

struct BpDataHeader {
  TopicId topic_id;
  MessageTypeId message_id;
  uint32_t size;
} __attribute__((packed));
const size_t bp_data_header_size = sizeof(BpDataHeader);

}  // namespace crps

#endif  // CRPS_BROKER_PROTOCOL_H