#ifndef VPT_LOGGING_HPP
#define VPT_LOGGING_HPP

#include <source_location>
#include <iostream>

#define vptFATAL(x) do { std::clog << "[FATAL]: " << x << "\n"; exit(1); } while(false)
#define vptWARN(x) do { std::clog << "[WARN]: " << x << "\n"; } while(false)
#define vptINFO(x) do { std::clog << "[INFO]: " << x << "\n"; } while(false)
#define vptDEBUG(x) do { \
  auto loc = std::source_location::current; \
  std::clog << "[DBG] (" << loc.file_name() << '(' << loc.line() << ':' << loc.column() << ") `" << loc.function_name() << "`" << "): " << x << std::endl; \
} while(false)

#endif // !VPT_LOGGING_HPP