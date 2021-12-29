#ifndef CRPS_NODE_H
#define CRPS_NODE_H

#include <list>
#include <memory>
#include <string>

#include "crps/defaults.h"
#include "crps/network.h"
#include "crps/publisher.h"
#include "crps/subscriber.h"

namespace crps {

class Node {
 private:
  std::string m_name;
  std::string m_broker_address;
  Network* m_network;
  Network::Protocol m_protocol;
  int16_t m_port;
  std::list<Publisher> m_publishers;
  std::list<Subscriber> m_subscribers;

 public:
  explicit Node(std::string p_name, std::string p_broker_address, Network* p_network,
                Network::Protocol p_protocol = Network::Protocol::UDP, int16_t p_port = broker_default_port);
  bool connect();
  Publisher* create_publisher(std::string p_topic_name, std::string p_type_name, MessageSize p_message_size,
                              TopicPriority p_topic_priority);
  Subscriber* create_subscriber(std::string p_topic_name, std::string p_type_name, MessageSize p_message_size,
                                SubscriberCallback p_callback, void* p_callback_user_data = nullptr);
};

}  // namespace crps

#endif  // CRPS_NODE_H