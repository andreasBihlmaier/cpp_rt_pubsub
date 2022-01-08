#ifndef CRPS_LOGGER_H
#define CRPS_LOGGER_H

#ifdef CRPS_LOGGER_NO_DEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CRPS_LOGGER_DEBUG(x, y)
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CRPS_LOGGER_DEBUG(x, y) x->logger().debug() y
#endif  // CRPS_LOGGER_NO_DEBUG

#include <functional>
#include <sstream>

namespace crps {

using WriteLogFunction = std::function<void(std::ostringstream& p_stream)>;

class Logger {
 public:
  enum class LogLevel {
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Fatal = 5,
  };

  static LogLevel global_log_level;

  explicit Logger(WriteLogFunction p_write_log) : m_log_level(LogLevel::Debug), m_write_log(std::move(p_write_log)) {
  }
  Logger(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = delete;
  ~Logger() {
    if (m_log_level >= global_log_level) {
      m_write_log(m_stream);
    }
  }

  std::ostringstream& log(LogLevel p_level);
  std::ostringstream& debug() {
    return log(LogLevel::Debug);
  }
  std::ostringstream& info() {
    return log(LogLevel::Info);
  }
  std::ostringstream& warn() {
    return log(LogLevel::Warn);
  }
  std::ostringstream& error() {
    return log(LogLevel::Error);
  }
  std::ostringstream& fatal() {
    return log(LogLevel::Fatal);
  }

 private:
  LogLevel m_log_level;
  std::ostringstream m_stream;
  WriteLogFunction m_write_log;
};

}  // namespace crps

#endif  // CRPS_LOGGER_H