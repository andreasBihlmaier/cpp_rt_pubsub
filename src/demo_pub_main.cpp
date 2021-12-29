#include <iostream>
#include <memory>

#include "crps/linux.h"
#include "crps/node.h"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  const crps::MessageSize message_size = 10;
  const crps::TopicPriority topic_priority = 1;

  auto os = std::make_unique<crps::LinuxOS>(true);
  auto network = std::make_unique<crps::LinuxNetwork>(os.get());

  auto node = std::make_unique<crps::Node>("test_pub", "127.0.0.1", network.get());
  auto* publisher = node->create_publisher("test_topic", "test_type", message_size, topic_priority);
  if (!node->connect()) {
    std::cout << "Publisher failed to connect. Exiting." << std::endl;
    return 1;
  }

  (void)publisher;  // TODO(ahb)

  return 0;
}