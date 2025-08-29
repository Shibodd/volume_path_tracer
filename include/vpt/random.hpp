#ifndef VPT_RANDOM_HPP
#define VPT_RANDOM_HPP

#include <cmath>

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