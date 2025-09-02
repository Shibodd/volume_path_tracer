#include <vpt/nanovdb_utils.hpp>

#include <vpt/majorant_transmittance_sampler.hpp>

namespace vpt {

MajorantTransmittanceSampler::MajorantTransmittanceSampler(
      RayMajorantIterator& it,
      RandomNumberGenerator& rng,
      const VolumeGrids& grids,
      const VolumeGrids::AccessorT& density_accessor,
      const VolumeGrids::AccessorT* temperature_accessor,
      float sigma_t)
  : m_T_maj(1.0f),
    m_sigma_t(sigma_t),
    m_rng(rng),
    m_iterator(it),
    m_grids(grids),
    m_density_sampler(density_accessor)
{
  if (temperature_accessor != nullptr)
    m_temperature_sampler.emplace(*temperature_accessor);
}

std::optional<MediumProperties> MajorantTransmittanceSampler::next() {
  while (true) {
    // Unless we're already sampling from a segment, we have to get a new one
    if (not m_segment) {
      m_segment = m_iterator.next();

      // No more segments left - quit
      if (not m_segment)
        return std::nullopt;

      // If we're stepping through empty space, just skip to the next segment
      if (m_segment->d_maj <= 0) {
        m_segment.reset();
        continue;
      }
    }

    assert(m_segment);

    // Compute sigma_maj for the current segment
    float sigma_maj = m_segment->d_maj * m_sigma_t;

    // Sample the next point
    float dt_m = sample_exponential(m_rng.uniform<float>(), sigma_maj);
    float t = m_segment->t0 + dt_m / m_iterator.idx_to_world_scale();

    // If the point lies within this segment
    if (t < m_segment->t1) {
      m_segment->t0 = t; // store start t of the next iteration

      m_T_maj *= std::exp(-dt_m * sigma_maj);

      // Retrieve the current sample point in density grid index space
      nanovdb::Vec3f point_density_grid = m_iterator.ray()(t);

      // Compute world position
      nanovdb::Vec3f point_world = m_grids.density().indexToWorldF(point_density_grid);
    
      float density = m_density_sampler(point_density_grid);
      if (density <= 0.0f)
        continue;

      float temperature = m_temperature_sampler? (*m_temperature_sampler)(m_grids.temperature().worldToIndexF(point_world)) : 0.0f;

      return MediumProperties {
        .point = nanovdb_to_eigen_f(point_world),
        .sigma_maj = sigma_maj,
        .density = density,
        .temperature = temperature,
        .ray_t = t
      };
    } else {
      // We're past the end of the current segment.

      // Update T_maj to account for the remaining space til the end of this segment
      float dt_m = (m_segment->t1 - m_segment->t0) * m_iterator.idx_to_world_scale();
      m_T_maj *= std::exp(-dt_m * sigma_maj);

      // Forget the segment so that we grab a new one at the next iteration
      m_segment.reset();
    }
  }

  std::unreachable();
}

} // namespace vpt