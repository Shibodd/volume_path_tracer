#include <vpt/worker.hpp>

namespace vpt {

void run(const Camera& camera, TileProvider& tp, Image<float, 3>& m_film, RandomNumberGenerator rng) {
  while (auto tok = tp.next()) {
    image_rect_t rect = tok.compute_rect();
    
    rng.begin_job(tok.jid());

    for (image_index_t y = 0; y < rect.size.y(); ++y) {
      for (image_index_t x = 0; x < rect.size.x(); ++x) {
        image_point_t pt = rect.start + image_point_t { x, y };

        std::remove_reference_t<decltype(m_film)>::value_t rgb_sample { 0.0f, 0.0f, 0.0f };

        Ray r = camera.generate_ray(pt);

        float t = r.intersect_sphere(Eigen::Vector3f{30.0f, 10.0f, 5.0f}, 1.0f);

        if (not std::isnan(t)) {
          rgb_sample = Eigen::Vector3f { 1.0f, 1.0f, 1.0f };
        }

        // sample lambda
        // sample Li
        // match to XYZ
        // transform to RGB
        // add to the film

        // Eigen::Vector3d rgb_sample { 0.1f, 0.1f, 0.1f };
        m_film.data()(pt.y(), pt.x()) += rgb_sample;
      }
    }
  }
}

} // namespace vpt