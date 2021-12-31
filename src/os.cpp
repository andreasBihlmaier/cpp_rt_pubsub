#include "crps/os.h"

#include <chrono>
#include <iomanip>

namespace crps {

std::ostringstream& Logger::log(LogLevel p_level) {
  auto now = std::chrono::system_clock::now();
  auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
  auto epoch_duration = now_ns.time_since_epoch();
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(epoch_duration);
  epoch_duration -= seconds;
  auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch_duration);
  const int digits_in_nanoseconds = 9;
  std::ios_base::fmtflags stream_fmt(m_stream.flags());
  m_stream << "[" << seconds.count() << "." << std::setw(digits_in_nanoseconds) << std::setfill('0')
           << nanoseconds.count() << "] ";
  m_stream.flags(stream_fmt);
  switch (p_level) {
    case LogLevel::Debug:
      m_stream << "[DEBUG] ";
      break;
    case LogLevel::Info:
      m_stream << "[INFO] ";
      break;
    case LogLevel::Warn:
      m_stream << "[WARN] ";
      break;
    case LogLevel::Error:
      m_stream << "[ERROR] ";
      break;
    case LogLevel::Fatal:
      m_stream << "[FATAL] ";
      break;
    default:
      break;
  }

  return m_stream;
}

Logger OS::logger() {
  return Logger(get_write_log_function());
}

}  // namespace crps