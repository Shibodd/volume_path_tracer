#ifndef VPT_COLOR_HPP
#define VPT_COLOR_HPP

#include <Eigen/Dense>

namespace vpt {

static inline Eigen::Vector3f xyz_to_linsrgb(const Eigen::Vector3f& xyz) {
  Eigen::Matrix3f m;
  
  m <<  3.240479, -1.537150, -0.498535,
       -0.969256,  1.875991,  0.041556,
        0.055648, -0.204043,  1.057311;
  
  return m * xyz;
}

static inline Eigen::Vector3f linsrgb_to_srgb(const Eigen::Vector3f linsrgb) {
  constexpr auto srgb = [](float x) {
    return x <= 0.0031308f ?
      (12.92f * x) :
      (1.055f * std::pow(x, 1.0f/2.4f) - 0.055f);
  };

  return Eigen::Vector3f {
    srgb(linsrgb.x()),
    srgb(linsrgb.y()),
    srgb(linsrgb.z())
  };
}

} // namespace vpt

#endif // !VPT_COLOR_HPP