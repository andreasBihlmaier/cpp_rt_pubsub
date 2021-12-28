#include <iostream>
#include <memory>

#include "crps/linux.h"
#include "crps/node.h"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  const crps::MessageTypeId message_type_id = 23;
  const crps::MessageSize message_size = 10;
  const crps::TopicPriority topic_priority = 1;

  auto network = std::make_unique<crps::LinuxNetwork>();

  auto node = std::make_unique<crps::Node>("test_pub", network.get());
  auto* publisher = node->create_publisher("test", message_type_id, message_size, topic_priority);
  if (!node->connect()) {
    std::cout << "Publisher failed to connect. Exiting." << std::endl;
    return 1;
  }

  (void)publisher;  // TODO(ahb)

  return 0;
}