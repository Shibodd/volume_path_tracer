#ifndef VPT_CAMERA_HPP
#define VPT_CAMERA_HPP

#include <Eigen/Dense>
#include <vpt/image.hpp>
#include <vpt/configuration.hpp>
#include <vpt/ray.hpp>

namespace vpt {

struct Camera {
  Camera(const CameraParameters& p, const image_size_t& im_sz);

  inline Ray generate_ray(const image_point_t& raster) const {
    Eigen::Vector3f raster_coords {
      raster.cast<float>().x() + 0.5f,
      raster.cast<float>().y() + 0.5f,
      0.0f
    };
    
    return {
      params().position,
      (m_raster_to_world_dir * raster_coords).normalized()
    };
  }

  const CameraParameters& params() const { return m_params; }
  const Eigen::Affine3f& raster_to_world_dir() const { return m_raster_to_world_dir; }
  const Eigen::Affine3f& screen_to_world_dir() const { return m_screen_to_world_dir; }
private:
  CameraParameters m_params;
  Eigen::Affine3f m_screen_to_world_dir;
  Eigen::Affine3f m_raster_to_world_dir;
};


} // namespace vpt

#endif // !VPT_CAMERA_HPP