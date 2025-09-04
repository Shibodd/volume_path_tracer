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

template <typename T, typename F>
static inline T lerp(const T& a, const T& b, const F& t) {
  return a + (b - a) * t;
}
/**
Generate an arbitrary frame of reference with one of the axis equal to v1

Modified PBRT's implementation.
// https://github.com/mmp/pbrt-v4
// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0
*/
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


/**
Adapted from PBRT.
// https://github.com/mmp/pbrt-v4
// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0
*/
static inline float henyey_greenstein(float cos_theta, float g) {
  float den = 1.0f + g*g + 2.0f * g * cos_theta;
  constexpr float inv_4_pi = static_cast<float>(std::numbers::inv_pi / 4.0);

  return inv_4_pi * (1.0f - g*g) / (den * std::sqrt(std::max(0.0f, den)));
}

} // namespace vpt

#endif // !VPT_UTILS_HPP