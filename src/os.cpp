#include "crps/os.h"

namespace crps {

Logger OS::logger() {
  return Logger(get_write_log_function());
}

}  // namespace crps