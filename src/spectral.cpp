#include <vpt/spectral.hpp>
#include "spectral_data/xyz.hpp"

namespace vpt {

namespace detail {
float planck_law(float lambda_m, float temperature_k) {
  if (temperature_k <= 0.0f) return 0.0f;
  
  constexpr float c = 299792458.f;
  constexpr float h = 6.62606957e-34f;
  constexpr float kb = 1.3806488e-23f;

  constexpr float num = 2 * h * c * c;
  float lambda5 = std::pow(lambda_m, 5);
  float exp = std::exp((h*c) / (lambda_m * kb * temperature_k));
  float den = lambda5 * (exp - 1);

  return num / den;
}
} // namespace detail

namespace spectra {
namespace cie_xyz {

const DenseSpectrum X = data::X;
const DenseSpectrum Y = data::Y;
const DenseSpectrum Z = data::Z;
const float Y_integral = data::Y_integral;

} // namespace cie_xyz
} // namespace spectra
} // namespace vpt