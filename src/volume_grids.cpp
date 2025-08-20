#include <utility>

// ffs... build your shit with warnings enabled pls
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#include <nanovdb/tools/CreatePrimitives.h>
#pragma GCC diagnostic pop

#include <nanovdb/io/IO.h>
#include <nanovdb/tools/GridBuilder.h>

#include <vpt/logging.hpp>
#include <vpt/volume_grids.hpp>

namespace vpt {

VolumeGrids::VolumeGrids(GridHandleT&& h_density, GridHandleT&& h_temperature)
  : m_density_handle(std::forward<GridHandleT>(h_density)),
    m_temperature_handle(std::forward<GridHandleT>(h_temperature)),
    m_density(m_density_handle.grid<float>()),
    m_temperature(m_temperature_handle.grid<float>())
{}

VolumeGrids VolumeGrids::generate_donut() {
  nanovdb::tools::build::FloatGrid temperature_grid(0.0f); // cold-ass donut
  
  return VolumeGrids {
    nanovdb::tools::createFogVolumeTorus(),
    nanovdb::tools::createNanoGrid(temperature_grid)
  };
}

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