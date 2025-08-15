#ifndef VPT_CAMERA_HPP
#define VPT_CAMERA_HPP

#include <Eigen/Dense>
#include <vpt/image.hpp>
#include <vpt/configuration.hpp>
#include <vpt/ray.hpp>

namespace vpt {

struct Camera {
  Camera(const CameraParameters& p, const image_size_t& im_sz);

  inline Ray generate_ray(const image_point_t& raster, const Eigen::Vector2f& jitter) const {
    Eigen::Vector2f rasterf2 = raster.cast<float>() + Eigen::Vector2f { 0.5f, 0.5f } + jitter;

    Eigen::Vector3f rasterf3 { rasterf2.x(), rasterf2.y(), 0.0f };
    
    return {
      params().position,
      (m_raster_to_world_dir * rasterf3).normalized()
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