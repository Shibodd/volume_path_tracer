#ifndef VPT_LOGGING_HPP
#define VPT_LOGGING_HPP

#include <fmt/core.h>

namespace vpt {

template <typename ...Args>
inline void fatal_error(fmt::format_string<Args...> format, Args&& ...args) { 
  fmt::println("[FATAL] {}", fmt::format(format, std::forward<Args>(args)...));
  exit(1);
}

template <typename ...Args>
inline void warn(fmt::format_string<Args...> format, Args&& ...args) { 
  fmt::println("[WARNING] {}", fmt::format(format, std::forward<Args>(args)...));
}

} // namespace vpt

#endif // !VOLUMEPATHTRACER_LOGGING_HPP