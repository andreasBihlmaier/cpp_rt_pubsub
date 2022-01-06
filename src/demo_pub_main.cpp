#include <memory>
#include <thread>

#include "crps/linux.h"
#include "crps/node.h"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  uint64_t value{0};
  const crps::MessageSize message_size = sizeof(value);
  const crps::TopicPriority topic_priority = 1;

  auto os = std::make_unique<crps::LinuxOS>(true);
  auto network = std::make_unique<crps::LinuxNetwork>(os.get());

  auto node = std::make_unique<crps::Node>("test_pub", "127.0.0.1", os.get(), network.get());
  auto* publisher = node->create_publisher("test_topic", "test_type", message_size, topic_priority);
  if (!node->connect()) {
    os->logger().error() << "Publisher failed to connect. Exiting.\n";
    return 1;
  }
  os->logger().info() << "Node: node name: '" << node->name() << ".\n";
  os->logger().info() << "Publisher: topic name: '" << publisher->topic_name()
                      << "'; topic ID: " << publisher->topic_id()
                      << "; topic priority: " << static_cast<int>(publisher->topic_priority())
                      << "; message type: " << publisher->message_type_name()
                      << "; message type ID: " << publisher->message_type_id()
                      << "; message size: " << publisher->message_size() << ".\n";

  for (;;) {
    publisher->publish(&value);
    if (!node->spin_while_work()) {
      os->logger().error() << "Node::spin_while_work failed. Exiting.\n";
      break;
    }
    value += 1;

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(500ms);
  }

  return 0;
}