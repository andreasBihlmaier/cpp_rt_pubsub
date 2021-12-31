#ifndef CRPS_LINUX_OS_H
#define CRPS_LINUX_OS_H

#include "crps/os.h"

namespace crps {

class LinuxOS final : public OS {
 public:
  explicit LinuxOS(bool p_flush = false);
  void write_log(std::ostringstream& p_stream) const;

 protected:
  WriteLogFunction get_write_log_function() override;

 private:
  bool m_flush;
};

}  // namespace crps

#endif  // CRPS_LINUX_OS_H