#ifndef CRPS_BROKER_H
#define CRPS_BROKER_H

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "crps/broker_protocol.h"
#include "crps/defaults.h"
#include "crps/message.h"
#include "crps/network.h"
#include "crps/os.h"
#include "crps/topic.h"

namespace crps {

using json = nlohmann::json;

class Broker {
 public:
  explicit Broker(std::string p_listen_address, OS* p_os, Network* p_network,
                  Network::Protocol p_protocol = Network::Protocol::UDP, int16_t p_port = broker_default_port);
  bool start();
  void spin();

 private:
  struct NodeInfo {
    std::string node_name;
  };

  bool verify_and_update_client_counter(const std::string& client, BpCounter counter);
  json process_control_request(const json& p_request);
  bool register_node(const std::string& p_node_name, BpNodeId* p_node_id);
  bool send_control_response(const std::string& p_address, const json& p_response);

  std::string m_listen_address;
  OS* m_os;
  Network* m_network;
  Network::Protocol m_protocol;
  std::unordered_map<std::string, BpCounter> m_client_counters;
  std::vector<NodeInfo> m_nodes;
};

}  // namespace crps

#endif  // CRPS_BROKER_H