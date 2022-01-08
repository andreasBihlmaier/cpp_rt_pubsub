#include <iostream>
#include <memory>

#include "crps/linux.h"
#include "crps/node.h"
#include "demo/parse_options.h"

void test_callback(void* p_message, crps::MessageSize /* UNUSED */, void* /* UNUSED */) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* value = reinterpret_cast<uint64_t*>(p_message);
  std::cout << "Got message: " << *value << "\n";
}

int main(int argc, char* argv[]) {
  std::string node_name{"test_sub"};
  std::string topic_name{"test_topic"};
  std::string message_type_name{"test_type"};
  crps::TopicPriority topic_priority{1};
  std::string listen_ip{"127.0.0.1"};

  auto options = parse_options(argc, argv);
  if (options.find("node_name") != options.end()) {
    node_name = options["node_name"];
  }
  if (options.find("topic_name") != options.end()) {
    topic_name = options["topic_name"];
  }
  if (options.find("message_type_name") != options.end()) {
    message_type_name = options["message_type_name"];
  }
  if (options.find("topic_priority") != options.end()) {
    topic_priority = std::stoi(options["topic_priority"]);
  }
  if (options.find("listen_ip") != options.end()) {
    listen_ip = options["listen_ip"];
  }

  // crps::Logger::global_log_level = crps::Logger::LogLevel::Info;

  const crps::MessageSize message_size = sizeof(uint64_t);

  auto os = std::make_unique<crps::LinuxOS>(true);
  auto network = std::make_unique<crps::LinuxNetwork>(os.get());

  auto node = std::make_unique<crps::Node>(node_name, listen_ip, os.get(), network.get());
  auto* subscriber =
      node->create_subscriber(topic_name, message_type_name, message_size, test_callback, nullptr, topic_priority);
  if (!node->connect()) {
    os->logger().error() << "Subscriber failed to connect. Exiting.\n";
    return 1;
  }
  os->logger().info() << "Node: node name: '" << node->name() << ".\n";
  os->logger().info() << "Subscriber: topic name: '" << subscriber->topic_name()
                      << "'; topic ID: " << subscriber->topic_id()
                      << "; topic priority: " << static_cast<int>(subscriber->topic_priority())
                      << "; message type: " << subscriber->message_type_name()
                      << "; message type ID: " << subscriber->message_type_id()
                      << "; message size: " << subscriber->message_size() << ".\n";

  node->spin();
  os->logger().error() << "Node::spin returned.\n";

  return 0;
}