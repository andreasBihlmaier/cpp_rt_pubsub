#ifndef CRPS_NODE_H
#define CRPS_NODE_H

#include <list>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "crps/broker_protocol.h"
#include "crps/defaults.h"
#include "crps/network.h"
#include "crps/os.h"
#include "crps/publisher.h"
#include "crps/subscriber.h"

namespace crps {

using json = nlohmann::json;

using NodeId = uint32_t;

class Node {
 private:
  std::string m_name;
  std::string m_broker_address;
  OS* m_os;
  Network* m_network;
  Network::Protocol m_protocol;
  int16_t m_port;
  NodeId m_node_id = 0;
  std::list<Publisher> m_publishers;
  std::list<Subscriber> m_subscribers;
  BpCounterType m_bp_counter = 0;

  bool register_node();
  json broker_rpc_blocking(const json& cmd);
  bool bp_send_control(const std::string& cmd);
  BpCounterType next_bp_counter();

 public:
  explicit Node(std::string p_name, std::string p_broker_address, OS* p_os, Network* p_network,
                Network::Protocol p_protocol = Network::Protocol::UDP, int16_t p_port = broker_default_port);
  bool connect();
  Publisher* create_publisher(std::string p_topic_name, std::string p_type_name, MessageSize p_message_size,
                              TopicPriority p_topic_priority);
  Subscriber* create_subscriber(std::string p_topic_name, std::string p_type_name, MessageSize p_message_size,
                                SubscriberCallback p_callback, void* p_callback_user_data = nullptr);
};

}  // namespace crps

#endif  // CRPS_NODE_H