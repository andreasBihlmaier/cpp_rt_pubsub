#ifndef CRPS_NODE_H
#define CRPS_NODE_H

#include <list>
#include <memory>
#include <string>

#include "crps/network.h"
#include "crps/publisher.h"
#include "crps/subscriber.h"

namespace crps {

class Node {
 private:
  std::string m_name;
  Network* m_network;
  std::list<Publisher> m_publishers;
  std::list<Subscriber> m_subscribers;

 public:
  explicit Node(std::string p_name, Network* p_network);
  bool connect();
  Publisher* create_publisher(std::string p_topic_name, MessageTypeId p_type_id, MessageSize p_message_size,
                              TopicPriority p_topic_priority);
  Subscriber* create_subscriber(std::string p_topic_name, MessageTypeId p_type_id, MessageSize p_message_size,
                                SubscriberCallback p_callback, void* p_callback_user_data = nullptr);
};

}  // namespace crps

#endif  // CRPS_NODE_H