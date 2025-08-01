#include <vpt/volume_grid.hpp>
#include <vpt/configuration.hpp>
#include <vpt/image.hpp>

#include <fmt/base.h>

int main() {
  vpt::Configuration cfg = vpt::read_configuration("configuration.json");

  vpt::VolumeGrids vol = vpt::VolumeGrids::read_from_file(cfg.volume_path);

  vpt::Image<unsigned char, 3> img(cfg.output_image.width, cfg.output_image.height);
  img.data().fill(decltype(img)::value_type(50, 50, 50));

  img.data()(50,0) = decltype(img)::value_type(100, 150, 200);

  img.save(cfg.output_image.path.c_str());

  return 0;
}