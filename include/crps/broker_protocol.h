#ifndef CRPS_BROKER_PROTOCOL_H
#define CRPS_BROKER_PROTOCOL_H

#include <cstdint>
#include <nlohmann/json.hpp>

#include "crps/message.h"
#include "crps/topic.h"

namespace crps {

using json = nlohmann::json;

// bp = broker protocol

using BpCounter = uint32_t;
using BpNodeId = uint32_t;

enum class BpType : uint8_t {
  Invalid = 0,
  Control = 1,
  Data = 2,
};

struct BpHeader {
  BpType type;
  BpCounter counter;
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

void bp_control_json_to_packet_buffer(const json& p_control_json, BpCounter counter,
                                      std::vector<unsigned char>* p_buffer);

}  // namespace crps

#endif  // CRPS_BROKER_PROTOCOL_H