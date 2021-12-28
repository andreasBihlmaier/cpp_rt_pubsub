#include <iostream>

#include "crps/subscriber.h"

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

  auto* subscriber = new crps::Subscriber{"test", message_type_id, message_size, test_callback};
  if (!subscriber->connect()) {
    std::cout << "Subscriber failed to connect. Exiting." << std::endl;
    return 1;
  }

  return 0;
}