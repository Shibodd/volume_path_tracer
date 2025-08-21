#include <vpt/camera.hpp>

namespace vpt {

static inline Eigen::Affine3f camera_to_world(const Eigen::Vector3f& pos, const Eigen::Vector3f& look, const Eigen::Vector3f& up) {
  Eigen::Vector3f dir = (look - pos).normalized();
  Eigen::Vector3f left = up.normalized().cross(dir);
  Eigen::Vector3f new_up = dir.cross(left);

  Eigen::Affine3f tf;
  tf.setIdentity();
  tf.linear().col(0) = left;
  tf.linear().col(1) = new_up;
  tf.linear().col(2) = dir;
  tf.translation() = pos;

  return tf;
}

/* Raster [0,res.w,0]x[0,res.h,0] to screen [-1,1,0]x[-1,1,0] */
static inline Eigen::Affine3f raster_to_screen(const image_size_t& bounds) {
  Eigen::Vector3f half_bounds { static_cast<float>(bounds.x()) / 2.0f, static_cast<float>(bounds.y()) / 2.0f, 0.0f };

  Eigen::Affine3f tf;
  tf.setIdentity();
  tf.linear() = (-half_bounds.cwiseInverse()).asDiagonal();
  tf.linear().bottomRightCorner<1,1>()[0] = 0.0f;
  tf.translation() = Eigen::Vector3f { 1.0f, 1.0f, 0.0f };

  return tf;
}

/* Normalized film [-1,1,0]x[-1,1,0] to camera assuming film is on the near plane, with near plane distance = 1 */ 
static inline Eigen::Affine3f screen_to_camera(float vfov, float ar) {
  float tan_vfov2 = std::tan(vfov/2);

  Eigen::Affine3f tf;
  tf.setIdentity();
  tf.linear() = Eigen::Vector3f { ar * tan_vfov2, tan_vfov2, 0.0f }.asDiagonal();
  tf.translation() = Eigen::Vector3f { 0.0f, 0.0f, 1.0f };

  return tf;
}

Camera::Camera(const CameraParameters& p, const image_size_t& im_sz)
  : m_params(p)
{
  float ar = static_cast<float>(im_sz.x()) / static_cast<float>(im_sz.y());
  float vfov_rad = float(std::numbers::pi) * p.vfov_deg / 180.0f;

  auto c2w = camera_to_world(p.position, p.look, p.up);
  auto s2c = screen_to_camera(vfov_rad, ar);
  auto r2s = raster_to_screen(im_sz);

  m_screen_to_world_dir = c2w.linear() * s2c;
  m_raster_to_world_dir = m_screen_to_world_dir * r2s;
}

} // namespace vpt