#include <memory>

#include "crps/broker.h"
#include "crps/linux.h"

int main(int argc, char* argv[]) {
  std::string listen_ip{"127.0.0.1"};
  if (argc == 3) {
    if (std::string(argv[1]) == "--listen_ip") {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      listen_ip = argv[2];                        // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
  }

  auto os = std::make_unique<crps::LinuxOS>(true);
  auto network = std::make_unique<crps::LinuxNetwork>(os.get());

  auto broker = std::make_unique<crps::Broker>(listen_ip, os.get(), network.get());
  if (!broker->start()) {
    os->logger().error() << "Broker failed to start. Exiting.\n";
    return 1;
  }

  broker->spin();

  return 0;
}