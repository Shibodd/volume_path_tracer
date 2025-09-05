#include <utility>
#include <optional>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-private-field"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <nanovdb/tools/CreatePrimitives.h>
#include <nanovdb/io/IO.h>
#include <nanovdb/tools/GridBuilder.h>
#pragma GCC diagnostic pop


#include <vpt/logging.hpp>
#include <vpt/volume_grids.hpp>

namespace vpt {

VolumeGrids::VolumeGrids(GridHandleT&& h_density)
  : m_density_handle(std::forward<GridHandleT>(h_density)),
    m_temperature_handle(std::nullopt),
    m_density(m_density_handle.grid<float>()),
    m_temperature(nullptr)
{}

VolumeGrids::VolumeGrids(GridHandleT&& h_density, GridHandleT&& h_temperature)
  : m_density_handle(std::forward<GridHandleT>(h_density)),
    m_temperature_handle(std::forward<GridHandleT>(h_temperature)),
    m_density(m_density_handle.grid<float>()),
    m_temperature(m_temperature_handle->grid<float>())
{}

VolumeGrids VolumeGrids::generate_donut() {
  return VolumeGrids { nanovdb::tools::createFogVolumeTorus() };
}

static inline std::optional<VolumeGrids::GridHandleT> nanovdb_try_read_grid(const std::filesystem::path& path, const std::string& grid_name) {
  try {
    return nanovdb::io::readGrid<VolumeGrids::GridHandleT::BufferType>(path, grid_name);
  } catch (const std::runtime_error& e) {
    vptWARN("NanoVDB failed to read grid \"" << grid_name << "\" from file \"" << path << "\": " << e.what());
  }
  return std::nullopt;
}

static inline VolumeGrids::GridHandleT nanovdb_read_grid_or_die(const std::filesystem::path& path, const std::string& grid_name) {
  auto grid = nanovdb_try_read_grid(path, grid_name);
  
  if (not grid) {
    vptFATAL("NanoVDB file " << path << " does not contain the \"" << grid_name << "\" grid.");
  }

  return std::move(*grid);
}

VolumeGrids VolumeGrids::read_from_file(const std::filesystem::path& path) {
  GridHandleT density = nanovdb_read_grid_or_die(path, "density");
  std::optional<GridHandleT> temperature = nanovdb_try_read_grid(path, "temperature");

  if (temperature) {
    return VolumeGrids { std::move(density), std::move(*temperature) };
  }

  return VolumeGrids { std::move(density) };
}

} // namespace vpt