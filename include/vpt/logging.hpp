#ifndef VPT_LOGGING_HPP
#define VPT_LOGGING_HPP

#include <source_location>
#include <iostream>

namespace logging {
namespace detail {

inline static std::ostream& write_loc_info(std::ostream& os, std::source_location loc = std::source_location::current()) {
  return os << loc.file_name() << '(' << loc.line() << ':' << loc.column() << ") `" << loc.function_name() << "`";
}
} // namespace detail
} // namespace logging

#define vptFATAL(x) do { std::clog << "[FATAL]: " << x << "\n"; exit(1); } while(false)
#define vptWARN(x) do { std::clog << "[WARN]: " << x << "\n"; } while(false)
#define vptINFO(x) do { std::clog << "[INFO]: " << x << "\n"; } while(false)
#define vptDEBUG(x) do { \
  std::clog << "[DBG] ("; \
  logging::detail::write_loc_info(std::clog); \
  std::clog << "): " << x << std::endl; \
} while(false)

#endif // !VPT_LOGGING_HPP