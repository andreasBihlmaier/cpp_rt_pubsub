#include <iostream>
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

  const crps::MessageTypeId message_type_id = 23;
  const crps::MessageSize message_size = 10;

  auto network = std::make_unique<crps::LinuxNetwork>();

  auto node = std::make_unique<crps::Node>("test_sub", network.get());
  auto* subscriber = node->create_subscriber("test", message_type_id, message_size, test_callback);
  if (!node->connect()) {
    std::cout << "Subscriber failed to connect. Exiting." << std::endl;
    return 1;
  }

  (void)subscriber;  // TODO(ahb)

  return 0;
}