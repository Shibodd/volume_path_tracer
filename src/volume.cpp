#include <vpt/volume.hpp>
#include <vpt/logging.hpp>
#include <vpt/utils.hpp>


namespace vpt {

/** @return The dimension at which HDDA should step to not skip any active leaf, when starting at the specified coordinate. */
inline static uint32_t get_hdda_dim(const RayMajorantIterator::CoordT& ijk, RayMajorantIterator::GridT::AccessorType& acc, const RayMajorantIterator::RayT& ray) {
  // Note that we only want to iterate up to the leaf level, not up to the voxel level.
  return std::max<uint32_t>(RayMajorantIterator::LEAF_DIM, acc.getDim(ijk, ray));
}

static inline nanovdb::math::Vec3f eigen_to_nanovdb_f(const Eigen::Vector3f& v) {
  return nanovdb::math::Vec3f { v.x(), v.y(), v.z() };
}

static inline nanovdb::math::Coord eigen_to_nanovdb_i(const Eigen::Vector3i& v) {
  return nanovdb::math::Coord { v.x(), v.y(), v.z() };
}

[[maybe_unused]]
static inline Eigen::Vector3i nanovdb_to_eigen_i(const nanovdb::math::Coord& v) {
  return Eigen::Vector3i { v.x(), v.y(), v.z() };
}

static inline Eigen::Vector3f nanovdb_to_eigen_f(const nanovdb::math::Vec3f& v) {
  return Eigen::Vector3f { v[0], v[1], v[2] };
}

float RayMajorantIterator::get_current_majorant() {
  auto ni = m_acc.getNodeInfo(m_hdda.voxel());
  if (ni.dim == LEAF_DIM) {
    // This is a leaf. The majorant is stored in maximum.
    return ni.maximum;
  } else { 
    // Not a leaf, meaning that it has constant value across its range.
    return m_acc.getValue(m_hdda.voxel());
  }
}

std::optional<RayMajorantIterator::Segment> RayMajorantIterator::next() {
  // We already left the bounding box. There's nothing left.
  if (m_hdda.time() >= m_hdda.maxTime()) {
    return std::nullopt;
  }

  Segment ans;
  ans.t0 = m_hdda.time() * m_scale;
  
  // Get this node's majorant value.
  float majorant = get_current_majorant();
  do {
    ans.majorant = majorant;

    if (not m_hdda.step()) {
      // We're leaving the bounding box - this is the last segment and we're done.

      // Veeery likely that the last segment has majorant=0... Don't return it if that's the case
      if (ans.majorant != 0) {
        ans.t1 = m_hdda.maxTime() * m_scale;
        return ans;
      } else {
        return std::nullopt;
      }
    }

    // Update the HDDA with the new step dimension.
    nanovdb::Coord next_ijk = m_ray(m_hdda.time() + 1.0001f).floor();
    uint32_t new_dim = get_hdda_dim(next_ijk, m_acc, m_ray);
    m_hdda.update(m_ray, new_dim);

    // Try to compute the majorant after the step. If it is the same, we'll bundle the two segments together.
    // This is useful when traversing empty space, because although efficient the stepping can be quite pessimistic.
    majorant = get_current_majorant();
  } while (majorant == ans.majorant);

  // We stepped - so the current HDDA time is the start of the next segment - equivalently, the end of the current one.
  ans.t1 = m_hdda.time() * m_scale;
  return ans;
}

std::optional<RayMajorantIterator> Volume::intersect(const vpt::Ray& ray) const {
  nanovdb::math::Ray<float> w_ray(eigen_to_nanovdb_f(ray.origin()), eigen_to_nanovdb_f(ray.direction()));
  nanovdb::math::Ray<float> i_ray = w_ray.worldToIndexF(m_grids.density());

  // Check intersection and clip the ray if there is one
  if (not i_ray.clip(m_grids.density().indexBBox())) {
    return std::nullopt; // no intersection -> no iterator
  }

  return RayMajorantIterator(i_ray, m_grids.density());
}

RayMajorantIterator::RayMajorantIterator(const RayT& ray, const GridT& density)
  : m_scale(1 / density.worldToIndexDirF(ray.dir()).length()),
    m_ray(ray), 
    m_acc(density.getAccessor()),
    m_hdda(m_ray, get_hdda_dim(m_ray.start().floor(), m_acc, m_ray))
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

Volume::Volume(const VolumeGrids& grids) : m_grids(grids) {
  fix_majorants_for_interpolation(m_grids.density(), 1);
}

Eigen::Vector3f Volume::world_to_density_index(const Eigen::Vector3f& world) const {
  return nanovdb_to_eigen_f(m_grids.density().worldToIndexF(eigen_to_nanovdb_f(world)));
}

void Volume::log_majorant_trace(const Ray& ray) const {
  if (ray.direction().cwiseAbs() != Eigen::Vector3f(1.0f, 0.0f, 0.0f)) {
    vptFATAL("For now this only supports ray in the +-X direction.");
  }

  std::optional<RayMajorantIterator> intersection = intersect(ray);

  std::ofstream log("majorant_trace.csv");
  print_csv(log, "X0", "X1", "T0", "T1", "Majorant") << '\n';

  if (intersection) {
    RayMajorantIterator iter = *intersection;
    while (auto seg_opt = iter.next()) {
      const RayMajorantIterator::Segment& segment = *seg_opt;

      Eigen::Vector3f p0 = world_to_density_index(ray.eval(segment.t0));
      Eigen::Vector3f p1 = world_to_density_index(ray.eval(segment.t1));
      print_csv(log, p0.x(), p1.x(), segment.t0, segment.t1, segment.majorant) << '\n';
    }
  }
}

void Volume::log_dda_trace(const Ray& vpt_ray) const {
  if (vpt_ray.direction().cwiseAbs() != Eigen::Vector3f(1.0f, 0.0f, 0.0f)) {
    vptFATAL("For now this only supports ray in the +-X direction.");
  }

  nanovdb::math::Ray<float> w_ray(eigen_to_nanovdb_f(vpt_ray.origin()), eigen_to_nanovdb_f(vpt_ray.direction()));
  w_ray.setMaxTime(10000.0f);

  nanovdb::math::Ray<float> ray = w_ray.worldToIndexF(m_grids.density());

  if (not ray.clip(m_grids.density().indexBBox())) {
    return;
  }

  auto acc = m_grids.density().tree().getAccessor();

  ray.setMinTime(ray.t0() - 16.0f);
  ray.setMaxTime(ray.t1() + 16.0f);

  std::ofstream out("dda_trace.csv");

  print_csv(out, "X", "Y", "Z", "Value", "Dim_getdim", "Dim_nodeinfo", "Active", "Maximum") << "\n";

  nanovdb::math::DDA<decltype(ray), nanovdb::math::Coord> dda(ray);
  do {
    nanovdb::Coord ijk = dda.voxel();
    float value = acc.getValue(ijk);

    auto ni = acc.getNodeInfo(ijk);
    uint32_t dim_getdim = acc.getDim(ijk, ray);
    uint32_t dim_nodeinfo = ni.dim;
    float max_value = ni.maximum;

    bool active = acc.isActive(ijk);

    print_csv(out, ijk.x(), ijk.y(), ijk.z(), value, dim_getdim, dim_nodeinfo, active, max_value) << std::endl;
  } while (dda.step());
}

} // namespace vpt