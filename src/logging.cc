#include "src/logging.hpp"

#ifdef SKITY_LOG
#include <spdlog/sinks/stdout_color_sinks.h>

namespace skity {

std::shared_ptr<spdlog::logger> g_logger;

void Log::Init() {
  g_logger = std::make_shared<spdlog::logger>(
      "skity", std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

  g_logger->set_pattern("%^[%T] %n: %v%$");

#ifdef SKITY_RELEASE
  g_logger->set_level(spdlog::level::err);
  g_logger->flush_on(spdlog::level::err);
#else
  g_logger->set_level(spdlog::level::debug);
  g_logger->flush_on(spdlog::level::debug);
#endif
}

std::shared_ptr<spdlog::logger> Log::GetLogger() {
  if (!g_logger) {
    Init();
  }

  return g_logger;
}

}  // namespace skity

#endif