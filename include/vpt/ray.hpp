#ifndef VPT_RAY_HPP
#define VPT_RAY_HPP

#include <Eigen/Dense>

namespace vpt {

struct Ray {
  Eigen::Vector3f origin;
  Eigen::Vector3f direction;
  inline auto eval(float t) const { return origin + direction * t; }

  float intersect_sphere(const Eigen::Vector3f& center, float r);
};

} // namespace vpt

#endif // !VPT_RAY_HPP