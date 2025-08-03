#ifndef VPT_CONFIGURATION_HPP
#define VPT_CONFIGURATION_HPP

#include <filesystem>

#include <vpt/image.hpp>

namespace vpt {

struct OutputImage {
  image_size_t size;
  std::filesystem::path path;
};

struct Configuration {
  std::filesystem::path volume_path;
  OutputImage output_image;
  image_size_t tile_size;
  unsigned int num_waves;
  unsigned int num_workers;
};

Configuration read_configuration(const std::filesystem::path& path);

} // namespace vpt

#endif // !VPT_CONFIGURATION_HPP