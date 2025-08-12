#ifndef VPT_WORKER_HPP
#define VPT_WORKER_HPP

#include <vpt/camera.hpp>
#include <vpt/tile_provider.hpp>
#include <vpt/image.hpp>
#include <vpt/random.hpp>

namespace vpt {

void run(const Configuration::DemoParameters& demo_cfg, const Camera& camera, TileProvider& tp, Image<float, 3>& m_film, RandomNumberGenerator rng);

} // namespace vpt

#endif // !VPT_RENDERER_HPP