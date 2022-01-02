#ifndef CRPS_OS_H
#define CRPS_OS_H

#include <arpa/inet.h>  // TODO(ahb) add abstraction/ifdef for ntoh* and ntoh*

#include <functional>
#include <sstream>

#include "crps/logger.h"

namespace crps {

class OS {
 public:
  Logger logger();

 protected:
  virtual WriteLogFunction get_write_log_function() = 0;
};

}  // namespace crps

#endif  // CRPS_OS_H