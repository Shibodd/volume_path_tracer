#ifndef VPT_SPECTRAL_HPP
#define VPT_SPECTRAL_HPP

#include <Eigen/Dense>
#include <vpt/utils.hpp>
#include <cmath>

namespace vpt {

namespace detail {
float planck_law(float lambda_m, float temp_k);
} // namespace detail

struct DenseSpectrum;

struct BlackbodyEmittedRadianceSpectrum {
  float operator()(float lambda_nm) const {
    return detail::planck_law(lambda_nm * 1e-9f, m_temperature);
  }

  BlackbodyEmittedRadianceSpectrum(float temperature)
    : m_temperature(temperature)
  {}

  // max_value(detail::planck_law(2.8977721e-3f / m_temperature, m_temperature))
private:
  float m_temperature;
};

struct DenseSpectrum {
  constexpr static unsigned int LAMBDA_MIN = 360;
  constexpr static unsigned int LAMBDA_MAX = 830;
  constexpr static size_t NUM_WAVELENGTHS = 1 + LAMBDA_MAX - LAMBDA_MIN;
  using data_t = std::array<float, NUM_WAVELENGTHS>;

  float operator()(unsigned int lambda) const {
    if (lambda < LAMBDA_MIN || lambda > LAMBDA_MAX)
      return 0.0f;

    return m_values[lambda - LAMBDA_MIN];
  }

  float operator[](size_t i) const { return m_values[i]; }

  DenseSpectrum(const data_t& data) : m_values(data) {}

private:
  data_t m_values;
};

namespace spectra {

namespace cie_xyz {
const extern DenseSpectrum X;
const extern DenseSpectrum Y;
const extern DenseSpectrum Z;
const extern float Y_integral;
} // namespace cie_xyz

} // namespace spectra

static inline float inner_product(const DenseSpectrum& a, const BlackbodyEmittedRadianceSpectrum& b) {
  float integral = 0.0f;
  for (unsigned int i = 0; i < DenseSpectrum::NUM_WAVELENGTHS; ++i)
    integral += a[i] * b(static_cast<float>(DenseSpectrum::LAMBDA_MIN + i));
  return integral;
}

static inline Eigen::Vector3f spectrum_to_xyz(const BlackbodyEmittedRadianceSpectrum& s) {
  return Eigen::Vector3f {
    inner_product(spectra::cie_xyz::X, s),
    inner_product(spectra::cie_xyz::Y, s),
    inner_product(spectra::cie_xyz::Z, s),
  } / spectra::cie_xyz::Y_integral;
}

} // namespace vpt

#endif // VPT_SPECTRAL_HPP