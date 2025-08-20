#ifndef VPT_VOLUME_HPP
#define VPT_VOLUME_HPP

#include <optional>

#include <nanovdb/math/Ray.h>
#include <nanovdb/math/HDDA.h>

#include <vpt/volume_grids.hpp>
#include <vpt/ray.hpp>

namespace vpt {

struct Volume {
  Volume(const VolumeGrids& grids);
private:
  const VolumeGrids& m_grids;
};

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

  static std::optional<RayMajorantIterator> create_from_intersection(const vpt::Ray& ray, const GridT& density);

private:
  RayMajorantIterator(const RayT& ray, const GridT& density);

  RayT m_ray;
  GridT::AccessorType m_acc;

  nanovdb::math::HDDA<RayT> m_hdda;
};

} // namespace vpt

#endif // !VPT_VOLUME_HPP