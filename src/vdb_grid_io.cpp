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
      vptFATAL("NanoVDB failed to read grid \"" << grid_name << "\" from file \"" << path << "\": " << e.what());
    }
    std::unreachable();
  };

  auto grid = try_read(path, grid_name);
  
  if (not grid) {
    vptFATAL("NanoVDB file " << path << " does not contain the \"" << grid_name << "\" grid.");
  }

  return grid;
}

VolumeGrids VolumeGrids::read_from_file(const std::filesystem::path& path) {
  return VolumeGrids {
    nanovdb_read_grid_or_die(path, "temperature"),
    nanovdb_read_grid_or_die(path, "density")
  };
}

} // namespace vpt