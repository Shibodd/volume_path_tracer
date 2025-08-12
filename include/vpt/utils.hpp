#ifndef VPT_UTILS_HPP
#define VPT_UTILS_HPP

#include <type_traits>

namespace vpt {

template <typename T>
requires std::is_integral_v<T>
constexpr static inline T ceildiv(T x, T y) {
  return x / y + (x % y != 0);
}

} // namespace vpt

#endif // !VPT_UTILS_HPP