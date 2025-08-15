#ifndef VPT_WORKER_HPP
#define VPT_WORKER_HPP

#include <vpt/camera.hpp>
#include <vpt/tile_provider.hpp>
#include <vpt/image.hpp>
#include <vpt/random.hpp>

namespace vpt {

struct Star {
  Eigen::Vector3f position;
  float radius;
  Eigen::Vector3f xyz;
};

void run(const std::vector<Star>& stars, const Camera& camera, TileProvider& tp, Image<float, 4>& m_film, RandomNumberGenerator rng);

} // namespace vpt

#endif // !VPT_RENDERER_HPP