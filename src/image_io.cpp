#include <fstream>

#include <spng.h>

#include <vpt/logging.hpp>
#include <vpt/image.hpp>

namespace vpt {
namespace detail {


struct SpngCtxDeleter {
  void operator()(spng_ctx* ptr) {
    spng_ctx_free(ptr);
  }
};

bool save_image(const std::filesystem::path& path, const char* data, int width, int height, int byte_depth) {
  std::unique_ptr<spng_ctx, SpngCtxDeleter> ctx(spng_ctx_new(SPNG_CTX_ENCODER));

  if (ctx == NULL) {
    vptWARN("Failed to create SPNG context!");
    return false;
  }

  struct spng_ihdr ihdr;
  memset(&ihdr, 0, sizeof(spng_ihdr));
  ihdr.width = width;
  ihdr.height = height;
  ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR;
  ihdr.bit_depth = byte_depth * 8;

  FILE* file = fopen(path.c_str(), "wb");
  if (file == nullptr) {
    vptWARN("Failed to open PNG output file \"" << path << "\"");
    return false;
  }


  int ec;

  if ((ec = spng_set_ihdr(ctx.get(), &ihdr)) != 0) {
    vptFATAL("spng_set_ihdr errored with errno " << ec << ": " << spng_strerror(ec));
    return false;
  }
  if ((ec = spng_set_png_file(ctx.get(), file)) != 0) {
    vptFATAL("spng_set_png_file errored with errno " << ec << ": " << spng_strerror(ec));
    return false;
  }
  if ((ec = spng_encode_image(ctx.get(), data, width * height * 3 * byte_depth, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE)) != 0) {
    vptWARN("Failed to encode PNG (errno " << ec << "): " << spng_strerror(ec));
    return false;
  }

  fclose(file);

  return true;
}

} // namespace detail
} // namespace vpt