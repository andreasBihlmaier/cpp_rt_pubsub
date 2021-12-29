#include "crps/linux_os.h"

#include <iostream>

namespace crps {

WriteLogFunction LinuxOS::get_write_log_function() {
  return [&](std::ostringstream& p_stream) { write_log(p_stream); };
}

LinuxOS::LinuxOS(bool p_flush) : m_flush(p_flush) {
}

void LinuxOS::write_log(std::ostringstream& p_stream) const {
  std::cout << p_stream.str();
  if (m_flush) {
    std::cout.flush();
  }
}

}  // namespace crps