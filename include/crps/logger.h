#ifndef CRPS_LOGGER_H
#define CRPS_LOGGER_H

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

  explicit Logger(WriteLogFunction p_write_log) : m_write_log(std::move(p_write_log)) {
  }
  Logger(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = delete;
  ~Logger() {
    m_write_log(m_stream);
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
  std::ostringstream m_stream;
  WriteLogFunction m_write_log;
};

}  // namespace crps

#endif  // CRPS_LOGGER_H