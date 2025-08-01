#ifndef VPT_CONFIGURATION_HPP
#define VPT_CONFIGURATION_HPP

#include <filesystem>

namespace vpt {

struct OutputImage {
  int width;
  int height;
  std::filesystem::path path;
};

struct Configuration {
  std::filesystem::path volume_path;
  OutputImage output_image;
};

Configuration read_configuration(const std::filesystem::path& path);

} // namespace vpt

#endif // !VPT_CONFIGURATION_HPP