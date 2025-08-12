#include <vpt/worker.hpp>
#include <vpt/spectral.hpp>

namespace vpt {

void run(const Configuration::DemoParameters& demo_cfg, const Camera& camera, TileProvider& tp, Image<float, 3>& m_film, RandomNumberGenerator rng) {
  while (auto tok = tp.next()) {
    image_rect_t rect = tok.compute_rect();
    
    rng.begin_job(tok.jid());

    // Compute XYZ of blackbody emitter
    
    vpt::BlackbodyEmittedRadianceSpectrum bb(demo_cfg.sphere_temperature);
    Eigen::Vector3f blackbody_xyz = spectrum_to_xyz(bb);

    for (image_index_t y = 0; y < rect.size.y(); ++y) {
      for (image_index_t x = 0; x < rect.size.x(); ++x) {
        image_point_t pt = rect.start + image_point_t { x, y };

        std::remove_reference_t<decltype(m_film)>::value_t sample { 0.0f, 0.0f, 0.0f };

        Ray r = camera.generate_ray(pt);

        float t = r.intersect_sphere(demo_cfg.sphere_position, demo_cfg.sphere_radius);

        if (not std::isnan(t)) {
          Eigen::Vector3f hitpos = r.eval(t);
          Eigen::Vector3f normal = (hitpos - demo_cfg.sphere_position).normalized(); 

          sample = blackbody_xyz * std::max(0.0f, -normal.dot(r.direction()));
        }

        // sample lambda
        // sample Li
        // match to XYZ
        // add to the film

        m_film.data()(pt.y(), pt.x()) += camera.params().imaging_ratio * sample;
      }
    }
  }
}

} // namespace vpt