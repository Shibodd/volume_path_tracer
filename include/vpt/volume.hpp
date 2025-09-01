#ifndef VPT_VOLUME_HPP
#define VPT_VOLUME_HPP

#include <optional>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#include <nanovdb/math/Ray.h>
#include <nanovdb/math/HDDA.h>
#include <nanovdb/math/SampleFromVoxels.h>
#pragma GCC diagnostic pop

#include <vpt/configuration.hpp>
#include <vpt/volume_grids.hpp>
#include <vpt/ray.hpp>

namespace vpt {

struct RayMajorantIterator {
  using GridT = VolumeGrids::GridT;
  using RayT = nanovdb::math::Ray<float>;
  using CoordT = nanovdb::math::Coord;

  static constexpr auto LEAF_DIM = GridT::LeafNodeType::DIM;

  struct Segment {
    float t0; // in voxel units
    float t1; // in voxel units
    float d_maj;
  };
  
  std::optional<Segment> next();
  RayMajorantIterator(const RayT& ray, const GridT& density, const GridT::AccessorType& density_accessor);

  const RayT& ray() const { return m_ray; }

  float idx_to_world_scale() const { return m_scale; }

private:
  float get_current_majorant();

  float m_scale;
  RayT m_ray;
  
  const GridT::AccessorType& m_acc;
  nanovdb::math::HDDA<RayT> m_hdda;
};

struct Volume {
  Volume(const VolumeGrids& grids, const VolumeParameters& params);

  void log_dda_trace(const Ray& vpt_ray, const VolumeGrids::AccessorT& density_accessor) const;
  void log_majorant_trace(const Ray& vpt_ray, const VolumeGrids::AccessorT& density_accessor) const;

  std::optional<RayMajorantIterator> intersect(const vpt::Ray& ray, const VolumeGrids::AccessorT& density_accessor) const;
  Eigen::Vector3f world_to_density_index(const Eigen::Vector3f& world) const;

  std::optional<VolumeGrids::AccessorT> make_temperature_accessor() const {
    if (m_grids.has_temperature())
      return m_grids.temperature().getAccessor();
    else
      return std::nullopt;
  }
  VolumeGrids::AccessorT make_density_accessor() const { return m_grids.density().getAccessor(); }

  // I really hate that these are here... but whatever - this class is already an almost useless wrapper
  const VolumeParameters& params() const { return m_params; }

  const VolumeGrids& grids() const { return m_grids; }

private:
  const VolumeGrids& m_grids;
  VolumeParameters m_params;
};

} // namespace vpt

#endif // !VPT_VOLUME_HPP