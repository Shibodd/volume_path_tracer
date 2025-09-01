#ifndef VPT_RANDOM_HPP
#define VPT_RANDOM_HPP

#include <span>
#include <cmath>
#include <numeric>

#include <Eigen/Dense>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#include <pcg/pcg_random.hpp>
#pragma GCC diagnostic pop

#include <vpt/hash.hpp>

namespace vpt {

/* exp(-ax) */
static inline float sample_exponential(float u, float a) {
  return -std::log(1 - u) / a;
}

template <typename T>
struct Choice {
  T value;
  float weight;
};

template <typename T>
static inline T sample_discrete(std::initializer_list<Choice<T>> choices, float u) {
  if (std::empty(choices))
    return T();

  float total_w = std::accumulate(choices.begin(), choices.end(), 0.0f, [](float acc, Choice<T> x) { return acc + x.weight; });
  
  u = u * total_w;

  auto end = choices.end();
  for (auto it = choices.begin(); it != end; ++it) {
    u -= it->weight;
    if (u <= 0)
      return it->value;
  }

  return std::prev(end)->value;
}

static inline float henyey_greenstein(float cos_theta, float g) {
  float g2 = std::pow(g, 2);

  float num = 1.0f - g2;
  float den = 1.0f + g2 + 2.0f * g * cos_theta;

  constexpr float INV_4PI = static_cast<float>(std::numbers::inv_pi) / 4.0f;
  return INV_4PI * num / (den * std::sqrt(std::max(0.0f, den)));
}

static inline Eigen::Vector3f sample_henyey_greenstein(const Eigen::Vector3f& w, const Eigen::Vector2f& u, float g) {
  // Sample direction in local spherical coordinates
  float g2 = std::pow(g, 2.0f);

  float cos_theta;
  if (std::abs(g) < 1e-3f)
    cos_theta = 1 - 2 * u.x();
  else
    cos_theta = 1.0f / (2.0f * g) * (1.0f + g2 - std::pow((1.0f - g2) / (1.0f + g - 2.0f * g * u.x()), 2.0f));

  float sin_theta = std::sqrt(std::max(0.0f, 1.0f - std::pow(cos_theta, 2.0f)));
  float phi = 2.0f * static_cast<float>(std::numbers::pi) * u.y();

  // Transform to local cartesian
  Eigen::Vector3f local(
    std::clamp(sin_theta, -1.0f, 1.0f) * std::cos(phi),
    std::clamp(sin_theta, -1.0f, 1.0f) * std::sin(phi),
    std::clamp(cos_theta, -1.0f, 1.0f)
  );
  local.normalize();

  // Make an arbitrary frame of reference with the Z axis = w
  Eigen::Vector3f x, y;
  Eigen::Vector3f z = w;
  coordinate_system(z, x, y);
  
  // Transform from local to world direction
  return local.x() * x + local.y() * y + local.z() * z;
}

struct RandomNumberGenerator {
  using engine = pcg32_fast;

  RandomNumberGenerator(engine::result_type seed)
    : m_seed(seed)
  { }

  void begin_job(size_t jid) {
    m_engine.seed(hash(m_seed, jid));
  }

  template<typename T>
  T uniform();

  template <>
  uint32_t uniform() {
    auto val = m_engine();
    static_assert(std::is_same_v<decltype(val), uint32_t>);
    return val;
  }

  template <>
  float uniform() {
    constexpr float ONE_MINUS_EPS = float(0x1.fffffep-1);
    return std::min<float>(ONE_MINUS_EPS, uniform<uint32_t>() * 0x1p-32f);
  }
private:
  engine m_engine;
  engine::result_type m_seed;
};

} // namespace vpt

#endif // !VPT_RANDOM_HPP