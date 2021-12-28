#include <iostream>

#include "crps/publisher.h"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  const crps::MessageTypeId message_type_id = 23;
  const crps::MessageSize message_size = 10;
  const crps::TopicPriority topic_priority = 1;

  auto* publisher = new crps::Publisher{"test", message_type_id, message_size, topic_priority};
  if (!publisher->connect()) {
    std::cout << "Publisher failed to connect. Exiting." << std::endl;
    return 1;
  }

  return 0;
}