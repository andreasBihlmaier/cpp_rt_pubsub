#include <memory>

#include "crps/broker.h"
#include "crps/linux.h"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  auto network = std::make_unique<crps::LinuxNetwork>();

  auto broker = std::make_unique<crps::Broker>("127.0.0.1", network.get());
  broker->start();

  return 0;
}