#ifndef VPT_RAY_HPP
#define VPT_RAY_HPP

#include <Eigen/Dense>

namespace vpt {

struct Ray {
  Ray(Eigen::Vector3f origin, Eigen::Vector3f direction) : m_origin(origin), m_direction(direction) {}

  inline auto eval(float t) const { return m_origin + m_direction * t; }
  
  const Eigen::Vector3f& origin() const { return m_origin; }
  const Eigen::Vector3f& direction() const { return m_direction; }

  float intersect_sphere(const Eigen::Vector3f& center, float r);
private:
  Eigen::Vector3f m_origin;
  Eigen::Vector3f m_direction;
};

} // namespace vpt

#endif // !VPT_RAY_HPP