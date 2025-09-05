#ifndef VPT_VOLUME_HPP
#define VPT_VOLUME_HPP

#include <vpt/nanovdb_utils.hpp>
#include <optional>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-private-field"
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
  
  struct DDAStep {
    int dim;
    float majorant;
    float t;
    Eigen::Vector3i voxel;
    Eigen::Vector3f hit;
  };
  
  void record_steps(std::vector<DDAStep>* dst) { m_step_record_dst = dst; }

private:
  void record_step() {
    if (m_step_record_dst != nullptr) {
      m_step_record_dst->push_back({
        .dim = m_dda.dim(),
        .majorant = m_majorant,
        .t = m_dda.time(),
        .voxel = nanovdb_to_eigen_i(m_dda.voxel()),
        .hit = nanovdb_to_eigen_f(m_ray(m_dda.time()))
      });
    }
  }

  void update_current_majorant();

  float m_scale;
  RayT m_ray;
  float m_majorant;

  const GridT::AccessorType& m_acc;
  nanovdb::math::HDDA<RayT> m_dda;
  std::vector<DDAStep>* m_step_record_dst;
};

struct Volume {
  Volume(const VolumeGrids& grids, const VolumeParameters& params);

  const Eigen::Vector3f& bounding_sphere_center() const { return m_bsphere_center; }
  float bounding_sphere_radius() const { return m_bsphere_radius; }

  void log_dda_trace(const Ray& vpt_ray, const VolumeGrids::AccessorT& density_accessor) const;
  void log_majorant_trace(const Ray& vpt_ray, const VolumeGrids::AccessorT& density_accessor) const;

  std::optional<RayMajorantIterator> intersect(const vpt::Ray& ray, const VolumeGrids::AccessorT& density_accessor) const;
  Eigen::Vector3f world_to_density_index(const Eigen::Vector3f& world) const;

  // I really hate that these are here... but whatever - this class is already an almost useless wrapper
  const VolumeParameters& params() const { return m_params; }

  const VolumeGrids& grids() const { return m_grids; }

private:
  Eigen::Vector3f m_bsphere_center;
  float m_bsphere_radius;
  const VolumeGrids& m_grids;
  VolumeParameters m_params;
};

} // namespace vpt

#endif // !VPT_VOLUME_HPP