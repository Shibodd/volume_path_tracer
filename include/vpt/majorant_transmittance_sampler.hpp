#ifndef VPT_MAJORANT_TRANSMITTANCE_SAMPLER
#define VPT_MAJORANT_TRANSMITTANCE_SAMPLER

#include <vpt/volume.hpp>
#include <vpt/random.hpp>

namespace vpt {

struct MediumProperties {
  Eigen::Vector3f point;
  float sigma_maj;
  float density;
};

struct MajorantTransmittanceSampler {
  MajorantTransmittanceSampler(
    RayMajorantIterator& it,
    RandomNumberGenerator& rng,
    const VolumeGrids::GridT& density_grid,
    const VolumeGrids::AccessorT& density_accessor,
    float sigma_t
  );

  float T_maj() const { return m_T_maj; }

  std::optional<MediumProperties> next();
private:
  float m_T_maj;

  float m_sigma_t;

  RandomNumberGenerator& m_rng;
  RayMajorantIterator m_iterator;

  const VolumeGrids::GridT& m_density_grid;
  nanovdb::math::SampleFromVoxels<VolumeGrids::AccessorT, 1> m_density_sampler;

  std::optional<RayMajorantIterator::Segment> m_segment;
};

} // namespace vpt

#endif // !VPT_MAJORANT_TRANSMITTANCE_SAMPLER