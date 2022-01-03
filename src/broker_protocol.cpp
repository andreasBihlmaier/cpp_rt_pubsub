#include "crps/broker_protocol.h"

#include "crps/os.h"

namespace crps {

void bp_control_json_to_packet_buffer(const json& p_control_json, BpCounter p_counter,
                                      std::vector<unsigned char>* p_buffer) {
  std::string control_string(p_control_json.dump());
  p_buffer->resize(bp_header_size + bp_control_header_size + control_string.size());
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* bp_header = reinterpret_cast<BpHeader*>(p_buffer->data());
  bp_header->type = BpType::Control;
  bp_header->counter = htonl(p_counter);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* bp_control_header = reinterpret_cast<BpControlHeader*>(&(*p_buffer)[bp_header_size]);
  bp_control_header->size = htonl(control_string.size());
  std::memcpy(&(*p_buffer)[bp_header_size + bp_control_header_size], &control_string[0], control_string.size());
}

BpType decode_packet_buffer(const unsigned char* p_packet_buffer, size_t p_packet_size, BpHeader* p_header,
                            BpControlHeader* p_control_header, BpDataHeader* p_data_header, json* p_control_json,
                            Logger* p_logger) {
  if (p_packet_size < bp_header_size) {
    if (p_logger != nullptr) {
      p_logger->error() << "Packet shorter than BpHeader. This indicates a serious issue!\n";
    }
    return BpType::Invalid;
  }

  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto* const header = reinterpret_cast<const BpHeader*>(p_packet_buffer);
    p_header->type = header->type;
    p_header->counter = ntohl(header->counter);
  }
  if (p_header->type == BpType::Control) {
    {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, cppcoreguidelines-pro-bounds-pointer-arithmetic)
      const auto* const control_header = reinterpret_cast<const BpControlHeader*>(p_packet_buffer + bp_header_size);
      p_control_header->size = ntohl(control_header->size);
    }
    uint32_t json_end_offset = bp_header_size + bp_control_header_size + p_control_header->size;
    if (json_end_offset > p_packet_size) {
      if (p_logger != nullptr) {
        p_logger->error() << "BpControlHeader::size (= " << p_control_header->size
                          << ") is wrong. End of JSON data would be at " << json_end_offset
                          << " which is beyond packet size (= " << p_packet_size
                          << "). This indicates corrupted packet data!\n";
      }
      return BpType::Invalid;
    }
    *p_control_json =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        json::parse(p_packet_buffer + bp_header_size + bp_control_header_size, p_packet_buffer + json_end_offset);
  } else if (p_header->type == BpType::Data) {
    // TODO(ahb)
    // remember to use nto* on BpDataHeader fields!
    (void)p_data_header;
    if (p_logger != nullptr) {
      p_logger->error() << "functionality not yet implemented in function " << __FUNCTION__ << "\n";  // NOLINT
    }
    return BpType::Invalid;
  }

  return p_header->type;
}

}  // namespace crps