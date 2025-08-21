#ifndef VPT_VOLUME_HPP
#define VPT_VOLUME_HPP

#include <optional>

#include <nanovdb/math/Ray.h>
#include <nanovdb/math/HDDA.h>

#include <vpt/volume_grids.hpp>
#include <vpt/ray.hpp>

namespace vpt {

struct RayMajorantIterator {
  using GridT = VolumeGrids::GridT;
  using RayT = nanovdb::math::Ray<float>;
  using CoordT = nanovdb::math::Coord;

  static constexpr auto LEAF_DIM = GridT::LeafNodeType::DIM;

  struct Segment {
    float t0;
    float t1;
    float majorant;
  };

  std::optional<Segment> next();
  RayMajorantIterator(const RayT& ray, const GridT& density);

private:
  float get_current_majorant();

  float m_scale;
  RayT m_ray;
  GridT::AccessorType m_acc;

  nanovdb::math::HDDA<RayT> m_hdda;
};

struct Volume {
  Volume(const VolumeGrids& grids);

  void log_dda_trace(const Ray& vpt_ray) const;
  void log_majorant_trace(const Ray& vpt_ray) const;

  std::optional<RayMajorantIterator> intersect(const vpt::Ray& ray) const;
  Eigen::Vector3f world_to_density_index(const Eigen::Vector3f& world) const;
private:
  const VolumeGrids& m_grids;
};

} // namespace vpt

#endif // !VPT_VOLUME_HPP