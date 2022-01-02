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

}  // namespace crps