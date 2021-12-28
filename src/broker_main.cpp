#include "crps/broker.h"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  auto* broker = new crps::Broker{"127.0.0.1"};
  broker->start();

  return 0;
}