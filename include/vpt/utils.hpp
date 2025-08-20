#ifndef VPT_UTILS_HPP
#define VPT_UTILS_HPP

#include <type_traits>
#include <iostream>

namespace vpt {

template <typename T>
requires std::is_integral_v<T>
constexpr static inline T ceildiv(T x, T y) {
  return x / y + (x % y != 0);
}

template <typename T>
static inline std::ostream& print_csv(std::ostream& os, const T& arg) {
  return os << arg;
}

template <typename T, typename... Args>
static inline std::ostream& print_csv(std::ostream& os, const T& first, const Args&... rest) {
  return print_csv(os << first << ",", rest...);
}

} // namespace vpt

#endif // !VPT_UTILS_HPP