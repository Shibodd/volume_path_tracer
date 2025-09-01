#ifndef VPT_UTILS_HPP
#define VPT_UTILS_HPP

#include <type_traits>
#include <iostream>
#include <Eigen/Dense>

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

/** Generate an arbitrary frame of reference with one of the axis equal to v1 */
static inline void coordinate_system(Eigen::Vector3f v1, Eigen::Vector3f& v2, Eigen::Vector3f& v3) {
  float sign = std::copysign(1.0f, v1.z());
  float a = -1.0f / (sign + v1.z());
  float b = v1.x() * v1.y() * a;

  v2.x() = 1.0f + sign * a * std::pow(v1.x(), 2.0f);
  v2.y() = sign * b;
  v2.z() = -sign * v1.x();

  v3.x() = b;
  v3.y() = sign + a * std::pow(v1.y(), 2.0f);
  v3.z() = -v1.y();
}

} // namespace vpt

#endif // !VPT_UTILS_HPP