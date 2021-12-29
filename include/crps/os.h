#ifndef CRPS_OS_H
#define CRPS_OS_H

#include <functional>
#include <iostream>  // TODO(ahb) rm
#include <sstream>

namespace crps {

using WriteLogFunction = std::function<void(std::ostringstream& p_stream)>;

class Logger {
 private:
  std::ostringstream m_stream;
  WriteLogFunction m_write_log;

 public:
  enum class LogLevel {
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Fatal = 5,
  };

  explicit Logger(WriteLogFunction p_write_log);
  Logger(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = delete;
  ~Logger();

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
};

class OS {
 protected:
  virtual WriteLogFunction get_write_log_function() = 0;

 public:
  Logger logger();
};

}  // namespace crps

#endif  // CRPS_OS_H