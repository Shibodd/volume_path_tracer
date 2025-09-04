#include <vpt/volume.hpp>
#include <vpt/logging.hpp>
#include <vpt/utils.hpp>

#include <vpt/nanovdb_utils.hpp>

namespace vpt {


/** -@return The dimension at which HDDA should step to not skip any active leaf, when starting at the specified coordinate. 
inline static uint32_t get_hdda_dim(const RayMajorantIterator::CoordT& ijk, const RayMajorantIterator::GridT::AccessorType& acc, const RayMajorantIterator::RayT& ray) {
  // Note that we only want to iterate up to the leaf level, not up to the voxel level.
  return std::max<uint32_t>(0, acc.getDim(ijk, ray));
}
*/


float RayMajorantIterator::get_current_majorant() {

  auto ijk = m_ray(m_dda.time()).floor();
  
  if (const auto* leaf = m_acc.probeLeaf(ijk)) {
    // This is a leaf. The majorant is stored in maximum.
    return leaf->getMax();
  } else { 
    // Not a leaf. It may still have a value which is constant value across its range.

    float value;
    if (m_acc.probeValue(ijk, value)) {
      return value;
    }
    
    return 0.0f;
  }
}

std::optional<RayMajorantIterator::Segment> RayMajorantIterator::next() {
  // We already left the bounding box. There's nothing left.
  if (m_dda.time() >= m_dda.maxTime()) {
    return std::nullopt;
  }

  Segment ans;
  ans.t0 = m_dda.time();
  
  // Get this node's majorant value.
  float majorant = get_current_majorant();
  do {
    ans.d_maj = majorant;

    if (not m_dda.step()) {
      // We're leaving the bounding box - this is the last segment and we're done.

      // Veeery likely that the last segment has majorant=0... Don't return it if that's the case
      if (ans.d_maj != 0) {
        ans.t1 = m_dda.maxTime();
        return ans;
      } else {
        return std::nullopt;
      }
    }

    // Update the HDDA with the new step dimension.
    // uint32_t new_dim = get_hdda_dim(m_ray(m_hdda.time() + 1.0001f).floor(), m_acc, m_ray);
    // m_hdda.update(m_ray, new_dim);

    // Try to compute the majorant after the step. If it is the same, we'll bundle the two segments together.
    // This is useful when traversing empty space, because although efficient the stepping can be quite pessimistic.
    majorant = get_current_majorant();
  } while (majorant == ans.d_maj);

  // We stepped - so the current HDDA time is the start of the next segment - equivalently, the end of the current one.
  ans.t1 = m_dda.time();
  return ans;
}

std::optional<RayMajorantIterator> Volume::intersect(const vpt::Ray& ray, const VolumeGrids::AccessorT& density_accessor) const {
  nanovdb::math::Ray<float> w_ray(eigen_to_nanovdb_f(ray.origin()), eigen_to_nanovdb_f(ray.direction()));
  nanovdb::math::Ray<float> i_ray = w_ray.worldToIndexF(m_grids.density());

  // Check intersection and clip the ray if there is one
  if (not i_ray.clip(m_grids.density().indexBBox())) {
    return std::nullopt; // no intersection -> no iterator
  }

  return RayMajorantIterator(i_ray, m_grids.density(), density_accessor);
}

RayMajorantIterator::RayMajorantIterator(const RayT& ray, const GridT& density, const GridT::AccessorType& density_accessor)
  : m_scale(1 / density.worldToIndexDirF(ray.dir()).length()),
    m_ray(ray), 
    m_acc(density_accessor),
    m_dda(m_ray)// , get_hdda_dim(ray.start().floor(), density_accessor, ray))
{
}

/**
  @brief Update the maximum value in each leaf to act as the majorant density, accounting for the effect of interpolation.
  @param order The size of the stencil used by the interpolator minus 1 divided by 2. For example, for the trilinear interpolator this is 1.
*/
static inline void fix_majorants_for_interpolation(VolumeGrids::GridT& density, unsigned int order) {
  /*
    TODO: OK, for leaves this is fine... What about the rest?
  */

  assert(order > 0);

  auto leaf_begin = density.tree().getFirstLeaf();
  auto leaf_end = leaf_begin + density.tree().nodeCount<VolumeGrids::GridT::LeafNodeType>();

  auto acc = density.getAccessor();

  // For each leaf
  for (auto& leaf : std::ranges::subrange(leaf_begin, leaf_end)) {
    // The maximum raw voxel data value in each leaf is already stored in the grid.    
    float majorant_density = leaf.getMax();

    /*
    However, it does not account for interpolation:
    Values near the edges, where the interpolator stencil leaves the current leaves, may be larger!
    In the following we account for interpolation.
    */

    constexpr auto LEAF_DIM = VolumeGrids::GridT::LeafNodeType::DIM;

    // Compute the leaf bounding box
    nanovdb::math::BBox<nanovdb::math::Coord> leaf_bbox(leaf.origin(), leaf.origin().offsetBy(LEAF_DIM-1));

    // Compute the interpolation AoE
    auto aoe_bbox = leaf_bbox.expandBy(order);

    // For each neighbouring leaf-sized bbox (don't look at the compiler output with -O3...)
    for (int i = -1; i <= 1; ++i) {
      for (int j = -1; j <= 1; ++j) {
        for (int k = -1; k <= 1; ++k) {
          if (i == 0 and j == 0 and k == 0)
            continue;

          // Compute the neighbour bbox
          auto neighbour_bbox = leaf_bbox;
          neighbour_bbox.translate(eigen_to_nanovdb_i(Eigen::Vector3i{ i, j, k } * LEAF_DIM));

          // Intersect the neighbour with the interpolator stencil AoE
          neighbour_bbox.intersect(aoe_bbox);
          
          // Compute the upper bound value at this intersection
          for (const nanovdb::math::Coord& c : neighbour_bbox) {
            // You COULD use a tighter upper bound... but is it really going to change much? (and do i even care)
            majorant_density = std::max(majorant_density, acc.getValue(c));
          }
        } 
      }
    }

    leaf.setMax(majorant_density);
  }
}

Volume::Volume(const VolumeGrids& grids, const VolumeParameters& params)
    : m_grids(grids), m_params(params) {

  nanovdb::Vec3f span = m_grids.density().worldBBox().max() - m_grids.density().worldBBox().min();
  m_bsphere_center = nanovdb_to_eigen_f(m_grids.density().worldBBox().min() + span / 2.0f);
  m_bsphere_radius = (span / 2).length();

  fix_majorants_for_interpolation(m_grids.density(), 1);
}

Eigen::Vector3f Volume::world_to_density_index(const Eigen::Vector3f& world) const {
  return nanovdb_to_eigen_f(m_grids.density().worldToIndexF(eigen_to_nanovdb_f(world)));
}

void Volume::log_majorant_trace(const Ray& ray, const VolumeGrids::AccessorT& density_accessor) const {
  std::optional<RayMajorantIterator> intersection = intersect(ray, density_accessor);

  std::ofstream log("majorant_trace.csv");
  print_csv(log, "X0", "Y0", "Z0", "X1", "Y1", "Z1", "T0", "T1", "Majorant") << '\n';

  if (intersection) {
    RayMajorantIterator iter = *intersection;
    while (auto seg_opt = iter.next()) {
      const RayMajorantIterator::Segment& segment = *seg_opt;

      Eigen::Vector3f p0 = world_to_density_index(ray.eval(segment.t0 * iter.idx_to_world_scale()));
      Eigen::Vector3f p1 = world_to_density_index(ray.eval(segment.t1 * iter.idx_to_world_scale()));
      print_csv(log, p0.x(), p0.y(), p0.z(), p1.x(), p1.y(), p1.z(), segment.t0 * iter.idx_to_world_scale(), segment.t1 * iter.idx_to_world_scale(), segment.d_maj) << '\n';
    }
  }
}

void Volume::log_dda_trace(const Ray& vpt_ray, const VolumeGrids::AccessorT& density_accessor) const {
  nanovdb::math::Ray<float> w_ray(eigen_to_nanovdb_f(vpt_ray.origin()), eigen_to_nanovdb_f(vpt_ray.direction()));
  w_ray.setMaxTime(10000.0f);

  nanovdb::math::Ray<float> ray = w_ray.worldToIndexF(m_grids.density());

  if (not ray.clip(m_grids.density().indexBBox())) {
    return;
  }

  ray.setMinTime(ray.t0() - 16.0f);
  ray.setMaxTime(ray.t1() + 16.0f);

  std::ofstream out("dda_trace.csv");

  print_csv(out, "X", "Y", "Z", "T", "Value", "Dim_getdim", "Dim_nodeinfo", "Active", "Maximum") << "\n";

  nanovdb::math::DDA<decltype(ray), nanovdb::math::Coord> dda(ray);
  do {
    nanovdb::Coord ijk = dda.voxel();
    float value = density_accessor.getValue(ijk);

    auto ni = density_accessor.getNodeInfo(ijk);
    uint32_t dim_getdim = density_accessor.getDim(ijk, ray);
    uint32_t dim_nodeinfo = ni.dim;
    float max_value = ni.maximum;

    bool active = density_accessor.isActive(ijk);

    print_csv(out, ijk.x(), ijk.y(), ijk.z(), dda.time(), value, dim_getdim, dim_nodeinfo, active, max_value) << std::endl;
  } while (dda.step());
}

} // namespace vpt