#include <memory>
#include <thread>

#include "crps/linux.h"
#include "crps/node.h"
#include "demo/parse_options.h"

int main(int argc, char* argv[]) {
  auto options = parse_options(argc, argv);
  std::string node_name{option_or_default(options, "node_name", "test_pub")};
  std::string topic_name{option_or_default(options, "topic_name", "test_topic")};
  std::string message_type_name{option_or_default(options, "message_type_name", "test_type")};
  crps::TopicPriority topic_priority{
      static_cast<crps::TopicPriority>(std::stoi(option_or_default(options, "topic_priority", "1")))};
  std::string listen_ip{option_or_default(options, "listen_ip", "127.0.0.1")};
  unsigned publish_period_ms{static_cast<unsigned>(std::stoul(option_or_default(options, "publish_period_ms", "500")))};

  uint64_t value{0};
  const crps::MessageSize message_size = sizeof(value);

  auto os = std::make_unique<crps::LinuxOS>(true);
  auto network = std::make_unique<crps::LinuxNetwork>(os.get());

  auto node = std::make_unique<crps::Node>(node_name, listen_ip, os.get(), network.get());
  auto* publisher = node->create_publisher(topic_name, message_type_name, message_size, topic_priority);
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

    std::this_thread::sleep_for(std::chrono::milliseconds(publish_period_ms));
  }

  return 0;
}