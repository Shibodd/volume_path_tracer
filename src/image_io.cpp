#include <fstream>

#include <rpng/rpng.h>

#include <vpt/logging.hpp>
#include <vpt/image.hpp>

namespace vpt {
namespace detail {

struct RpngDeleter {
  void operator()(char* ptr) { RPNG_FREE(ptr); }
};

bool save_image(const std::filesystem::path& path, const char* data, int width, int height, int color_channels, int bit_depth) {
  int sz = 0;
  std::unique_ptr<char, RpngDeleter> buffer(
    rpng_save_image_to_memory(data, width, height, color_channels, bit_depth, &sz)
  ); 

  if (buffer == NULL || sz == 0) {
    warn("Failed to serialize {}x{}, {}-bit {}-channel image to PNG!", width, height, color_channels, bit_depth);
    return false;
  }

  std::ofstream out(path);

  if (!out.is_open()) {
    warn("Failed to open the output file \"{}\"!", path.c_str());
    return false;
  }

  out.write(buffer.get(), sz);

  out.close();
  
  if (out.fail()) {
    warn("Failed to write {} bytes to the output file!", sz);
    return false;
  }

  return true;
}

} // namespace detail
} // namespace vpt