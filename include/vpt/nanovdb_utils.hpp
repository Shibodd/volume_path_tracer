#ifndef NANOVDB_UTILS_HPP
#define NANOVDB_UTILS_HPP

#include <Eigen/Dense>
#include <nanovdb/math/Math.h>

static inline nanovdb::math::Vec3f eigen_to_nanovdb_f(const Eigen::Vector3f& v) {
  return nanovdb::math::Vec3f { v.x(), v.y(), v.z() };
}

static inline nanovdb::math::Coord eigen_to_nanovdb_i(const Eigen::Vector3i& v) {
  return nanovdb::math::Coord { v.x(), v.y(), v.z() };
}

static inline Eigen::Vector3i nanovdb_to_eigen_i(const nanovdb::math::Coord& v) {
  return Eigen::Vector3i { v.x(), v.y(), v.z() };
}

static inline Eigen::Vector3f nanovdb_to_eigen_f(const nanovdb::math::Vec3f& v) {
  return Eigen::Vector3f { v[0], v[1], v[2] };
}

#endif // !NANOVDB_UTILS_HPP