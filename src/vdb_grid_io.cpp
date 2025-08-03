#include <utility>

#include <nanovdb/io/IO.h>

#include <vpt/logging.hpp>
#include <vpt/volume_grid.hpp>

namespace vpt {

static inline VolumeGrids::GridHandleT nanovdb_read_grid_or_die(const std::filesystem::path& path, const std::string& grid_name) {
  constexpr auto try_read = [](const std::filesystem::path& path, const std::string& grid_name) {
    try {
      return nanovdb::io::readGrid<VolumeGrids::GridHandleT::BufferType>(path, grid_name);
    } catch (const std::runtime_error& e) {
      logging::fatal_error("NanoVDB failed to read grid \"{}\" from file \"{}\": {}", grid_name, path.c_str(), e.what());
    }
    std::unreachable();
  };

  auto grid = try_read(path, grid_name);
  
  if (not grid)
    logging::fatal_error("NanoVDB file {} does not contain the \"{}\" grid.", path.c_str(), grid_name);

  return grid;
}

VolumeGrids VolumeGrids::read_from_file(const std::filesystem::path& path) {
  return VolumeGrids {
    nanovdb_read_grid_or_die(path, "temperature"),
    nanovdb_read_grid_or_die(path, "density")
  };
}

} // namespace vpt