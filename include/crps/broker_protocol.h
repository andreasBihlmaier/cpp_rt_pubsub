#ifndef CRPS_BROKER_PROTOCOL_H
#define CRPS_BROKER_PROTOCOL_H

#include <arpa/inet.h>  // TODO(ahb) add abstraction/ifdef for ntoh* and ntoh*

#include <cstdint>
#include <nlohmann/json.hpp>

#include "crps/logger.h"
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
  MessageTypeId message_type_id;
  uint32_t size;
} __attribute__((packed));
const size_t bp_data_header_size = sizeof(BpDataHeader);

void bp_control_json_to_packet_buffer(const json& p_control_json, BpCounter p_counter,
                                      std::vector<unsigned char>* p_buffer);
void bp_fill_data_header(BpCounter p_counter, TopicId p_topic_id, MessageTypeId p_message_type_id,
                         uint32_t p_message_size, void* p_buffer);

// p_header is valid if returned value != BpType::Invalid
// p_control_header and p_control_json is valid if returned value == BpType::Control
// p_data_header is valid if returned value == BpType::Data
[[nodiscard]] BpType decode_packet_buffer(const unsigned char* p_packet_buffer, size_t p_packet_size,
                                          BpHeader* p_header, BpControlHeader* p_control_header,
                                          BpDataHeader* p_data_header, json* p_control_json,
                                          Logger* p_logger = nullptr);

}  // namespace crps

#endif  // CRPS_BROKER_PROTOCOL_H