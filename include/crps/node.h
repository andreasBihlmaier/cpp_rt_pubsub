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

class Node {
 public:
  enum class SpinOnceResult {
    Invalid = 0,
    Success = 1,
    NoWork = 2,
    Error = 3,
  };

  explicit Node(std::string p_name, std::string p_broker_host, OS* p_os, Network* p_network,
                Network::Protocol p_protocol = Network::Protocol::UDP, int16_t p_port = broker_default_port);
  bool connect();
  Publisher* create_publisher(std::string p_topic_name, std::string p_type_name, MessageSize p_message_size,
                              TopicPriority p_topic_priority);
  Subscriber* create_subscriber(std::string p_topic_name, std::string p_type_name, MessageSize p_message_size,
                                SubscriberCallback p_callback, void* p_callback_user_data = nullptr,
                                TopicPriority p_topic_priority = dontcare_topic_priority);
  bool spin();
  bool spin_while_work();
  SpinOnceResult spin_once();
  [[nodiscard]] std::string name() const {
    return m_name;
  }

 private:
  struct TopicInfo {
    std::list<unsigned> publishers;
    std::list<unsigned> subscribers;
  };

  struct TopicsComparator {
    bool operator()(const TopicPriority& lhs, const TopicPriority& rhs) const {
      return lhs > rhs;
    }
  };

  bool register_node();
  bool register_message_type(const std::string& p_message_type_name, MessageSize p_message_size,
                             MessageTypeId* p_message_type_id);
  bool register_topic(const std::string& p_topic_name, MessageTypeId p_message_type_id, TopicPriority p_topic_priority,
                      TopicId* p_topic_id, TopicPriority* p_actual_topic_priority = nullptr);
  bool register_publisher(Publisher* p_publisher);
  bool register_subscriber(Subscriber* p_subscriber);
  json broker_rpc_blocking(const json& p_request);
  bool send_control(const json& p_request);
  bool send_data(TopicId p_topic_id, MessageTypeId p_message_type_id, void* p_buffer, size_t p_buffer_size);
  [[nodiscard]] BpCounter next_bp_counter();
  [[nodiscard]] uint32_t rpc_id() const;

  std::string m_name;
  std::string m_broker_address;
  OS* m_os;
  Network* m_network;
  Network::Protocol m_protocol;
  BpNodeId m_node_id = 0;
  std::vector<Publisher> m_publishers;
  std::vector<Subscriber> m_subscribers;
  std::map<TopicPriority, TopicInfo, TopicsComparator> m_topics;  // sorted by highest to lowest priority
  BpCounter m_bp_counter = 0;
};

}  // namespace crps

#endif  // CRPS_NODE_H