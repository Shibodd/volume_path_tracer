#ifndef VPT_TILEPROVIDER_HPP
#define VPT_TILEPROVIDER_HPP

#include <mutex>
#include <vpt/image.hpp>

namespace vpt {

struct TileProvider {
  using tile_index_t = unsigned int;
  using tile_point_t = Eigen::Vector2<tile_index_t>;
  using tile_size_t = tile_point_t;
  using wave_index_t = unsigned int;

  struct token {
    image_rect_t compute_rect() { return m_tp.compute_tile_rect(m_idx); }
    bool valid() const { return m_idx != INVALID_IDX; }
    size_t wave() const { return m_wave_idx; }
    size_t jid() const { return m_jid; }
    operator bool() const { return valid(); }

    ~token() {
      if (valid()) {
        m_tp.m_tile_wave[m_idx] = m_wave_idx;
        m_tp.m_tile_wave[m_idx].notify_all();
      }
    }
    token(const token&) = delete;
    token& operator=(const token&) = delete;

    // We don't need moving for now...
    token(token&&) = delete;
    token& operator=(token&&) = delete;

  private:
    static constexpr tile_index_t INVALID_IDX = std::numeric_limits<tile_index_t>::max();

    friend TileProvider;

    static token invalid(TileProvider& tp) {
      return token { tp, INVALID_IDX, 0, 0 };
    }

    token(TileProvider& tp, unsigned int idx, wave_index_t wave_idx, size_t jid)
      : m_idx(idx), m_tp(tp), m_wave_idx(wave_idx), m_jid(jid)
    { }

    tile_index_t m_idx;
    TileProvider& m_tp;
    wave_index_t m_wave_idx;
    size_t m_jid;
  };

  friend token;

  TileProvider(const image_size_t& img_size, wave_index_t waves, const image_size_t& tile_size);

  token next();
  void stop_at_next_wave();
  void stop_now();

  void reset_eta() {
    m_start_t = std::chrono::steady_clock::now();
  }

  std::chrono::duration<float> eta() const {
    float progress = progress_ratio();
    float avg_progress_rate = progress / std::chrono::duration<float>(std::chrono::steady_clock::now() - m_start_t).count();
    return std::chrono::duration<float>((1 - progress) / avg_progress_rate);
  }

  unsigned int progress() const {
    return static_cast<unsigned int>(progress_ratio() * 100.0f);
  }

private:
  float progress_ratio() const {
    size_t max_jid = m_wave_start_monitor.requested_waves * m_tile_wave.size();
    return static_cast<float>(m_job_idx) / static_cast<float>(max_jid);
  }

  image_rect_t compute_tile_rect(tile_index_t tile_idx) const;
  bool wave_should_be_processed(wave_index_t idx);

  struct wave_start_monitor {
    std::mutex mtx;
    wave_index_t requested_waves;
    wave_index_t max_wave_idx;

    explicit wave_start_monitor(wave_index_t requested_waves)
      : requested_waves(requested_waves),
        max_wave_idx(0) 
    {}
  } m_wave_start_monitor;

  std::chrono::steady_clock::time_point m_start_t;

  bool m_force_stop;

  image_size_t m_img_size;
  image_size_t m_tile_size;

  tile_size_t m_num_tiles;

  std::atomic_size_t m_job_idx;
  std::vector<std::atomic<wave_index_t>> m_tile_wave;
};

} // namespace vpt

#endif // !VPT_RENDERER_HPP