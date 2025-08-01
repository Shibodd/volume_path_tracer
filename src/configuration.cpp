#include <glaze/glaze.hpp>

#include <vpt/configuration.hpp>
#include <vpt/logging.hpp>

namespace vpt {

Configuration read_configuration(const std::filesystem::path& path) {
  std::string buf;

  Configuration ans;
  glz::error_ctx err = glz::read_file_json(ans, "configuration.json", buf);
  if (err)
    vpt::fatal_error("Failed to read configuration file {}: {}", path.c_str(), glz::format_error(err, buf));

  return ans;
}

} // namespace vpt