#ifndef VPT_CONFIGURATION_HPP
#define VPT_CONFIGURATION_HPP

#include <filesystem>

namespace vpt {

struct Configuration {
  std::filesystem::path volume_path;
};

Configuration read_configuration(const std::filesystem::path& path);

} // namespace vpt

#endif // !VPT_CONFIGURATION_HPP