#include <vpt/color.hpp>
#include <vpt/spectral.hpp>
#include <vpt/utils.hpp>

namespace vpt {

constexpr unsigned int N_BREAKPOINTS = 500;
constexpr float RESOLUTION = 50000.0f / N_BREAKPOINTS;

constexpr float TEMP_MAX = (N_BREAKPOINTS - 1) * RESOLUTION;

static std::array<Eigen::Vector3f, N_BREAKPOINTS> bb_xyz;

static inline float idx_to_temp(int idx) {
  return (idx - 1) * RESOLUTION;
}

void init_blackbody_radiation_xyz() {
  for (unsigned int i = 0; i < N_BREAKPOINTS; ++i) {
    bb_xyz[i] = spectrum_to_xyz(BlackbodyEmittedRadianceSpectrum(idx_to_temp(i)));
  }
}


Eigen::Vector3f blackbody_radiation_xyz(float temperature_k) {
  if (not std::isfinite(temperature_k)) {
    return Eigen::Vector3f::Constant(std::numeric_limits<float>::signaling_NaN());
  }

  if (temperature_k <= 0.0f) {
    return Eigen::Vector3f::Zero();
  }

  if (temperature_k >= TEMP_MAX) {
    return spectrum_to_xyz(BlackbodyEmittedRadianceSpectrum(temperature_k));
  }

  // Find the idx with temp <= temperature_k
  int dn_idx = static_cast<int>(temperature_k / RESOLUTION);
  while (temperature_k <= idx_to_temp(dn_idx - 1))
    --dn_idx;
  while (temperature_k >= idx_to_temp(dn_idx + 1))
    ++dn_idx;

  float dn_temp = idx_to_temp(dn_idx);
  if (temperature_k == dn_temp)
    return bb_xyz[dn_idx];

  // Lerp between lower and upper
  float t = (temperature_k - idx_to_temp(dn_idx)) / RESOLUTION;
  return lerp(bb_xyz[dn_idx], bb_xyz[dn_idx+1], t);
}

}