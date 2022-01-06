#include <memory>

#include "crps/linux.h"
#include "crps/node.h"

void test_callback(void* p_message, crps::MessageSize p_message_size, void* /* UNUSED */) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* value = reinterpret_cast<uint64_t*>(p_message);
  (void)value;
  (void)p_message_size;

  // TODO(ahb)
}

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  const crps::MessageSize message_size = sizeof(uint64_t);
  const crps::TopicPriority topic_priority = 1;

  auto os = std::make_unique<crps::LinuxOS>(true);
  auto network = std::make_unique<crps::LinuxNetwork>(os.get());

  auto node = std::make_unique<crps::Node>("test_sub", "127.0.0.1", os.get(), network.get());
  auto* subscriber =
      node->create_subscriber("test_topic", "test_type", message_size, test_callback, nullptr, topic_priority);
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