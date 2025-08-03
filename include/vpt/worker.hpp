#ifndef VPT_WORKER_HPP
#define VPT_WORKER_HPP

#include <vpt/tile_provider.hpp>

namespace vpt {

struct Worker {
  Worker(TileProvider& tp);

  void run();

private:
  TileProvider& m_tp;
};

} // namespace vpt

#endif // !VPT_RENDERER_HPP