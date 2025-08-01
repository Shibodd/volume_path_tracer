#ifndef VPT_VOLUMEGRID_HPP
#define VPT_VOLUMEGRID_HPP

#include <filesystem>

#include <nanovdb/GridHandle.h>

namespace vpt {

struct VolumeGrids {
  using GridHandleT = nanovdb::GridHandle<nanovdb::HostBuffer>;

  GridHandleT density;
  GridHandleT temperature;

  static VolumeGrids read_from_file(const std::filesystem::path& path);
};

} // namespace vpt

#endif // !VPT_VOLUMEGRID_HPP