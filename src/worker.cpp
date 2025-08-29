#include <vpt/worker.hpp>
#include <vpt/spectral.hpp>
#include <vpt/majorant_transmittance_sampler.hpp>

namespace vpt {

void run(const WorkerParameters& params, const Volume& vol, const Camera& camera, TileProvider& tp, Image<float, 4>& m_film, RandomNumberGenerator rng) {
  auto density_acc = vol.make_density_accessor();
  auto temperature_acc = vol.make_temperature_accessor();

  while (auto tok = tp.next()) {
    image_rect_t rect = tok.compute_rect();
    
    rng.begin_job(tok.jid());

    for (image_index_t y = 0; y < rect.size.y(); ++y) {
      for (image_index_t x = 0; x < rect.size.x(); ++x) {
        image_point_t pt = rect.start + image_point_t { x, y };

        if (params.single_pixel.enabled) {
          if (params.single_pixel.coord != pt) {
            continue;
          }

          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        Eigen::Vector3f sample = decltype(sample)::Zero();
        float w = 1.0;

        Eigen::Vector2f jitter { rng.uniform<float>(), rng.uniform<float>() };
        jitter *= params.use_jitter? 0.5 : 0.0;

        Ray r = camera.generate_ray(pt, jitter);

        // vol.log_dda_trace(r);
        // vol.log_majorant_trace(r);

        if (auto intersection = vol.intersect(r, density_acc)) {
          MajorantTransmittanceSampler sampler(
            *intersection,
            rng,
            density_acc,
            temperature_acc,
            0.1f,
            0.1f
          );

          while (sampler.next()) {}

          sample = decltype(sample)::Ones() * sampler.T_maj();
        }
        
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