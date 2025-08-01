#include <vpt/volume_grid.hpp>
#include <vpt/configuration.hpp>

#include <fmt/base.h>

#include <CImg.h>

namespace cimg = cimg_library;

int main() {
  vpt::Configuration cfg = vpt::read_configuration("configuration.json");

  vpt::VolumeGrids vol = vpt::VolumeGrids::read_from_file(cfg.volume_path);

  cimg::CImg<float> img(500,400,1,3,0);
  // cimg::CImgDisplay main_disp(img,"The Image");

  img.save(cfg.output_image.path.c_str());
  

  return 0;
}