#include <mutex>

#include <vpt/tile_provider.hpp>
#include <vpt/logging.hpp>

namespace vpt {

template <typename T>
requires std::is_integral_v<T>
static inline T ceildiv(T x, T y) {
  return x / y + (x % y != 0);
}

TileProvider::TileProvider(const image_size_t& img_size, wave_index_t waves, const image_size_t& tile_size)
  : m_wave_start_monitor(waves),
    m_force_stop(false),
    m_img_size(img_size),
    m_tile_size(tile_size),
    m_num_tiles(
      ceildiv(img_size.x(), tile_size.x()),
      ceildiv(img_size.y(), tile_size.y())
    ),
    m_job_idx(0),
    m_tile_wave(m_num_tiles.x() * m_num_tiles.y())
{}

TileProvider::token TileProvider::next() {
  size_t job_idx = m_job_idx.fetch_add(1, std::memory_order_relaxed);

  wave_index_t wave_idx = 1 + (job_idx / m_tile_wave.size());
  tile_index_t tile_idx = job_idx % m_tile_wave.size();

  if (m_force_stop or not wave_should_be_processed(wave_idx))
    return token::invalid(*this);

  /* For the possible but improbable scenario where a thread gets stuck on a tile in an old wave...
      Having a dense representation of the tiles really reeks of "you're doing something wrong".
      But hey, it's fast.
  */
  while (not m_force_stop) {
    wave_index_t tile_wave = m_tile_wave[tile_idx].load(std::memory_order_relaxed);

    /*
    Threads will set tile_wave to wave_idx AFTER they finish running.
    Because only one thread gets to run a specific tile/wave pair:
    - if tile_wave == wave_idx - 1, then this tile has been fully processed in the previous wave.
    - if tile_wave < wave_idx - 1, then this tile is still being processed in one of the previous waves.
    */

    // If the tile wave index is the one before ours, then we can take it - it's not busy.
    if (tile_wave == wave_idx - 1)
      break;

    assert(tile_wave < wave_idx - 1);
    
    vptWARN("TILE PROVIDER: Need to process wave " << wave_idx << " for tile " << tile_idx << ", but it's stuck in wave " << tile_wave << "...");

    // The tile is still being processed in an old wave. We need to wait for our turn.
    m_tile_wave[tile_idx].wait(tile_wave, std::memory_order_relaxed);
  }

  if (m_force_stop) {
    return token::invalid(*this);
  }

  return token(*this, tile_idx, wave_idx);
}

bool TileProvider::wave_should_be_processed(wave_index_t idx) {
  // Fast path. If the wave has already started, we are good to go.
  if (idx <= m_wave_start_monitor.max_wave_idx)
    return true;
  
  std::unique_lock lock(m_wave_start_monitor.mtx);

  // Same check as above, but after locking the mutex.
  if (idx <= m_wave_start_monitor.max_wave_idx)
    return true;

  // So, this thread is starting a new wave.
  // Are we supposed to process this wave?
  if (idx > m_wave_start_monitor.requested_waves) {
    return false;
  }

  vptINFO("Starting wave " << idx);

  // Register the fact that this wave started.
  m_wave_start_monitor.max_wave_idx = idx;

  return true;
}

image_rect_t TileProvider::compute_tile_rect(tile_index_t tile_idx) const {
  tile_point_t x0_tile = {
    tile_idx % m_num_tiles.x(),
    tile_idx / m_num_tiles.x()
  };
  
  image_point_t x0 = x0_tile.cast<image_index_t>().cwiseProduct(m_tile_size);
  image_size_t sz = (m_img_size - x0).cwiseMin(m_tile_size);

  return image_rect_t { x0, sz };
}
  
void TileProvider::stop_at_next_wave() {
  std::unique_lock lock(m_wave_start_monitor.mtx);
  m_wave_start_monitor.requested_waves = m_wave_start_monitor.max_wave_idx;
}
void TileProvider::stop_now() { m_force_stop = true; }


} // namespace vpt