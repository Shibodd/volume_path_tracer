#include <vpt/nanovdb_utils.hpp>

#include <vpt/majorant_transmittance_sampler.hpp>

namespace vpt {

MajorantTransmittanceSampler::MajorantTransmittanceSampler(
      RayMajorantIterator& it,
      RandomNumberGenerator& rng,
      const VolumeGrids::AccessorT& density_accessor,
      const VolumeGrids::AccessorT& temperature_accessor,
      float sigma_t)
  : m_T_maj(1.0f),
    m_sigma_t(sigma_t),
    m_rng(rng),
    m_iterator(it),
    m_density_sampler(density_accessor),
    m_temperature_sampler(temperature_accessor)
{}

std::optional<MediumProperties> MajorantTransmittanceSampler::next() {
  while (true) {
    // Unless we're already sampling from a segment, we have to get a new one
    if (not m_segment) {
      m_segment = m_iterator.next();

      // No more segments left - quit
      if (not m_segment)
        return std::nullopt;
    }

    assert(m_segment);

    // Compute sigma_maj for the current segment
    float sigma_maj = m_segment->d_maj * m_sigma_t;

    // Sample the next point
    float dt = sample_exponential(m_rng.uniform<float>(), sigma_maj);
    float t = m_segment->t0 + dt;

    // If the point lies within this segment
    if (t < m_segment->t1) {
      m_segment->t0 = t; // store start t of the next iteration

      m_T_maj *= std::exp(-dt * sigma_maj);

      // Sample the volume at this point.
      nanovdb::Vec3f point = m_iterator.ray()(t);
      float density = m_density_sampler(point);
      return MediumProperties {
        .point = nanovdb_to_eigen_f(point),
        .sigma_maj = sigma_maj,
        .density = density,
        .temperature = m_temperature_sampler(point)
      };
    } else {
      // We're past the end of the current segment.

      // Update T_maj to account for the remaining space til the end of this segment
      float dt = m_segment->t1 - m_segment->t0;
      m_T_maj *= std::exp(-dt * sigma_maj);

      // Forget the segment so that we grab a new one at the next iteration
      m_segment.reset();
    }
  }

  std::unreachable();
}

} // namespace vpt