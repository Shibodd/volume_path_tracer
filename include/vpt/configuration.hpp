#ifndef VPT_CONFIGURATION_HPP
#define VPT_CONFIGURATION_HPP

#include <filesystem>

#include <vpt/image.hpp>

namespace vpt {

struct OutputImage {
  image_size_t size;
  std::filesystem::path path;
};

struct CameraParameters {
  Eigen::Vector3f position;
  Eigen::Vector3f look;
  Eigen::Vector3f up;

  float vfov_deg;
  float imaging_ratio;
};

struct WorkerParameters {
  struct SinglePixelMode {
    bool enabled;
    image_point_t coord;
  } single_pixel;

  bool use_jitter;
  Eigen::Vector3f ambient_light;
  float ambient_light_multiplier;
  unsigned int max_depth;
};

struct VolumeParameters {
  float henyey_greenstein_g;
  float le_scale;
  float sigma_a;
  float sigma_s;
  float temperature_offset;
  float temperature_scale;
};

struct Configuration {
  OutputImage output_image;
  image_size_t tile_size;
  unsigned int num_waves;
  unsigned int num_workers;
  CameraParameters camera_parameters;
  WorkerParameters worker_parameters;
  std::filesystem::path volume_path;
  VolumeParameters volume_parameters;
};

Configuration read_configuration(const std::filesystem::path& path);

} // namespace vpt

#endif // !VPT_CONFIGURATION_HPP