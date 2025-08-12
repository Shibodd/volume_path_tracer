#include <vpt/ray.hpp>

namespace vpt {

float Ray::intersect_sphere(const Eigen::Vector3f& center, float r) {
  const float& dx = direction().x();
  const float& dy = direction().y();
  const float& dz = direction().z();

  const float& x0 = origin().x();
  const float& y0 = origin().y();
  const float& z0 = origin().z();

  const float& cx = center.x();
  const float& cy = center.y();
  const float& cz = center.z();

  float a = dx*dx + dy*dy + dz*dz;
  float b = 2*dx*(x0-cx) + 2*dy*(y0-cy) + 2*dz*(z0-cz);
  float c = cx*cx + cy*cy + cz*cz + x0*x0 + y0*y0 + z0*z0 +
            -2*(cx*x0 + cy*y0 + cz*z0) - r*r;

  float delta = b * b - 4*a*c;
  if (delta < 0)
    return std::numeric_limits<float>::quiet_NaN();

  return (-b - std::sqrt(delta)) / (2 * a);
}

} // namespace vpt