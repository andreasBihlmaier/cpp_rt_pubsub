#include <iostream>
#include <memory>

#include "crps/broker.h"
#include "crps/linux.h"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  auto os = std::make_unique<crps::LinuxOS>(true);
  auto network = std::make_unique<crps::LinuxNetwork>(os.get());

  auto broker = std::make_unique<crps::Broker>("127.0.0.1", network.get());
  if (!broker->start()) {
    std::cout << "Broker failed to start. Exiting." << std::endl;
    return 1;
  }

  broker->spin();

  return 0;
}