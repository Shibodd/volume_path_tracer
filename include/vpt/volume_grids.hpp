#ifndef VPT_VOLUMEGRIDS_HPP
#define VPT_VOLUMEGRIDS_HPP

#include <filesystem>
#include <optional>

#include <nanovdb/GridHandle.h>

namespace vpt {

struct VolumeGrids {
  using GridHandleT = nanovdb::GridHandle<nanovdb::HostBuffer>;
  using GridT = nanovdb::NanoGrid<float>;

  VolumeGrids(GridHandleT&& density);
  VolumeGrids(GridHandleT&& density, GridHandleT&& temperature);

  inline GridT& density() const { return *m_density; }
  inline GridT& temperature() const { return *m_temperature; }
  inline bool has_temperature() const { return m_temperature != nullptr; }

  static VolumeGrids read_from_file(const std::filesystem::path& path);
  static VolumeGrids generate_donut();

private:
  GridHandleT m_density_handle;
  std::optional<GridHandleT> m_temperature_handle;

  GridT* m_density;
  GridT* m_temperature;
  
  GridHandleT m_majorant_handle;
};

} // namespace vpt

#endif // !VPT_VOLUMEGRIDS_HPP