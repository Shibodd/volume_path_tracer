#include <glaze/glaze.hpp>

#include <vpt/configuration.hpp>
#include <vpt/logging.hpp>

namespace vpt {

Configuration read_configuration(const std::filesystem::path& path) {
  std::string buf;

  Configuration ans;
  
  glz::error_ctx err = glz::read_file_json<glz::opts {
    .error_on_missing_keys = true
  }>(ans, "configuration.json", buf);

  if (err) {
    vptFATAL("Failed to read configuration file \"" << path << "\": " << glz::format_error(err, buf));
  }

  return ans;
}

} // namespace vpt