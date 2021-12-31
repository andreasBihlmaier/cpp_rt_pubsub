#ifndef CRPS_OS_H
#define CRPS_OS_H

#include <functional>
#include <sstream>

#include "crps/logger.h"

namespace crps {

class OS {
 protected:
  virtual WriteLogFunction get_write_log_function() = 0;

 public:
  Logger logger();
};

}  // namespace crps

#endif  // CRPS_OS_H