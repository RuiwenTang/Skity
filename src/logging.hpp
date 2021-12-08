#ifndef SKITY_SRC_LOGGING_HPP
#define SKITY_SRC_LOGGING_HPP

#include <memory>
#ifdef SKITY_LOG
#include <spdlog/spdlog.h>
#endif

#ifdef SKITY_LOG

namespace skity {
class Log {
 public:
  static void Init();

  static std::shared_ptr<spdlog::logger> GetLogger();
};
}  // namespace skity

#define LOG_INFO(...) skity::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) skity::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) skity::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_DEBUG(...) skity::Log::GetLogger()->debug(__VA_ARGS__)

#else

#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_DEBUG(...)

#endif

#endif  // SKITY_SRC_LOGGING_HPP