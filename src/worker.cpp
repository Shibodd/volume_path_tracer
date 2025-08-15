#include <vpt/worker.hpp>
#include <vpt/spectral.hpp>

namespace vpt {

void run(const Camera& camera, TileProvider& tp, Image<float, 4>& m_film, RandomNumberGenerator rng) {
  while (auto tok = tp.next()) {
    image_rect_t rect = tok.compute_rect();
    
    rng.begin_job(tok.jid());

    for (image_index_t y = 0; y < rect.size.y(); ++y) {
      for (image_index_t x = 0; x < rect.size.x(); ++x) {
        image_point_t pt = rect.start + image_point_t { x, y };

        Eigen::Vector3f sample { 0.0f, 0.0f, 0.0f };
        float w = 1.0;

        Eigen::Vector2f jitter { rng.uniform<float>(), rng.uniform<float>() };
        jitter *= 0.5;
        
        Ray r = camera.generate_ray(pt, jitter);

        (void)r;

        // sample lambda
        // sample Li
        // match to XYZ
        // add to the film
        m_film.data()(pt.y(), pt.x()).topRows<3>() += camera.params().imaging_ratio * sample;
        m_film.data()(pt.y(), pt.x()).w() += w;
      }
    }
  }
}

} // namespace vpt