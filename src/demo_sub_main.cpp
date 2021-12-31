#include <memory>

#include "crps/linux.h"
#include "crps/node.h"

void test_callback(void* p_message, crps::MessageSize p_message_size, void* p_user_data) {
  (void)p_message;
  (void)p_message_size;
  (void)p_user_data;

  // TODO(ahb)
}

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  const crps::MessageSize message_size = 10;

  auto os = std::make_unique<crps::LinuxOS>(true);
  auto network = std::make_unique<crps::LinuxNetwork>(os.get());

  auto node = std::make_unique<crps::Node>("test_sub", "127.0.0.1", os.get(), network.get());
  auto* subscriber = node->create_subscriber("test_topic", "test_type", message_size, test_callback);
  if (!node->connect()) {
    os->logger().error() << "Subscriber failed to connect. Exiting.\n";
    return 1;
  }

  (void)subscriber;  // TODO(ahb)

  return 0;
}