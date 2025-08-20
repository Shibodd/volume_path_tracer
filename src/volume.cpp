#include <vpt/volume.hpp>


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

/*
static inline Eigen::Vector3i nanovdb_to_eigen_i(const nanovdb::Coord& v) {
  return Eigen::Vector3i { v.x(), v.y(), v.z() };
}
*/

std::optional<RayMajorantIterator::Segment> RayMajorantIterator::next() {
  Segment ans;
  ans.t0 = m_hdda.time();

  /*

  // Setup the HDDA with the initial step dimension
  // acc.getDim is somewhat of a misnomer: it returns the dimension in which you have to step in order to not skip any active value.
  nanovdb::math::Coord first_ijk = ray.start().floor();
  nanovdb::math::HDDA hdda(ray, );  

  // Handle the first node, as we could very well already have hit an active node
  handle_node(hdda.voxel());

  while (hdda.step()) {
    handle_node(hdda.voxel());

    // Update the HDDA with the new step dimension.
    nanovdb::Coord next_ijk = ray(hdda.time() + 1.0001f).floor();
    uint32_t new_dim = std::max<uint32_t>(8, acc.getDim(next_ijk, ray));
    hdda.update(ray, new_dim);
  }
  */

  if (m_hdda.dim() == LEAF_DIM && m_acc.isActive(m_hdda.voxel())) {
    
  }

  return ans;
}

std::optional<RayMajorantIterator> RayMajorantIterator::create_from_intersection(const vpt::Ray& ray, const GridT& density) {
  nanovdb::math::Ray<float> w_ray(eigen_to_nanovdb_f(ray.origin()), eigen_to_nanovdb_f(ray.direction()));
  nanovdb::math::Ray<float> i_ray = w_ray.worldToIndexF(density);

  // Check intersection and clip the ray if there is one
  if (not i_ray.clip(density.indexBBox())) {
    return std::nullopt; // no intersection -> no iterator
  }

  return RayMajorantIterator(i_ray, density);
}

RayMajorantIterator::RayMajorantIterator(const RayT& ray, const GridT& density)
  : m_ray(ray), 
    m_acc(density.getAccessor()),
    m_hdda(m_ray, get_hdda_dim(m_ray.start().floor(), m_acc, m_ray))
{}


/**
  @brief Update the maximum value in each leaf to act as the majorant density, accounting for the effect of interpolation.
  @param order The size of the stencil used by the interpolator minus 1 divided by 2. For example, for the trilinear interpolator this is 1.
*/
static inline void fix_majorants_for_interpolation(VolumeGrids::GridT& density, unsigned int order) {
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
            // You COULD use a better upper bound... but is it really going to change much? (and do i even care)
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

} // namespace vpt