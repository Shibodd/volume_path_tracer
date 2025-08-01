#include <vpt/volume_grid.hpp>
#include <vpt/configuration.hpp>

#include <fmt/base.h>

int main() {
  vpt::Configuration cfg = vpt::read_configuration("configuration.json");
  vpt::VolumeGrids vol = vpt::VolumeGrids::read_from_file(cfg.volume_path);

  fmt::println("Density gridsize: {}", vol.density.gridSize());
  fmt::println("Temperature gridsize: {}", vol.temperature.gridSize());

  return 0;
}