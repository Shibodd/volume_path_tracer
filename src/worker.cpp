#include <vpt/worker.hpp>
#include <vpt/spectral.hpp>
#include <vpt/majorant_transmittance_sampler.hpp>

namespace vpt {

enum class ScatterEvent {
  Null,
  Absorption,
  Scatter
};

template <bool Enabled>
struct Logger {
  void new_ray(const Ray& r) {
    if constexpr (Enabled)
      print_csv(out, "new_ray", r.origin().x(), r.origin().y(), r.origin().z(), r.direction().x(), r.direction().y(), r.direction().z()) << '\n';
  }
  void sampled_point(const MediumProperties& props) {
    if constexpr (Enabled)
      print_csv(out, "sampled_point", props.point.x(), props.point.y(), props.point.z(), props.density) << '\n';
  }
  void null() {
    if constexpr (Enabled)
      print_csv(out, "null") << '\n';
  }
  void scatter_terminated() {
    if constexpr (Enabled)
      print_csv(out, "scatter_terminated") << '\n';
  }
  void scatter(const Ray& r) {
    if constexpr (Enabled)
      print_csv(out, "scatter", r.origin().x(), r.origin().y(), r.origin().z(), r.direction().x(), r.direction().y(), r.direction().z()) << '\n';
  }
  void absorbed() {
    if constexpr (Enabled)
      print_csv(out, "absorbed") << '\n';
  }

  Logger() {
    if constexpr (Enabled)
      out = std::ofstream("log.csv");
  }

private:
  std::ofstream out;
};


void run(const WorkerParameters& params, const Volume& vol, const Camera& camera, TileProvider& tp, Image<float, 4>& m_film, RandomNumberGenerator rng) {
  auto density_acc = vol.make_density_accessor();
  auto temperature_acc = vol.make_temperature_accessor();

  Logger<false> logger;

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

          // std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        /*
          Based on PBRT's SimpleVolPathIntegrator
          https://github.com/mmp/pbrt-v4
        */

        Eigen::Vector2f jitter { rng.uniform<float>(), rng.uniform<float>() };
        jitter *= params.use_jitter? 0.5 : 0.0;

        Ray r = camera.generate_ray(pt, jitter);
        logger.new_ray(r);

        // vol.log_dda_trace(r, density_acc);
        // vol.log_majorant_trace(r, density_acc);

        Eigen::Vector3f L = decltype(L)::Zero();
        bool terminated = false;

        for (unsigned int depth = 0; depth < params.max_depth; ++depth) {
          bool scattered = false;

          auto intersection = vol.intersect(r, density_acc);
          if (not intersection)
            break;
          
          // Sample on the current ray
          MajorantTransmittanceSampler sampler(
            *intersection,
            rng,
            vol.grids(),
            density_acc,
            temperature_acc? &*temperature_acc : nullptr,
            vol.params().sigma_a + vol.params().sigma_s
          );
          while (auto props = sampler.next()) {
            logger.sampled_point(*props);

            float p_a = props->density * vol.params().sigma_a / props->sigma_maj;
            float p_s = props->density * vol.params().sigma_s / props->sigma_maj;
            
            float p_n = std::max<float>(1.0f - p_a - p_s, 0.0f);

            ScatterEvent event = sample_discrete<ScatterEvent>({
              { ScatterEvent::Null, p_n },
              { ScatterEvent::Absorption, p_a },
              { ScatterEvent::Scatter, p_s },
            }, rng.uniform<float>());

            if (event == ScatterEvent::Null) {
              logger.null();
              continue;
            } else if (event == ScatterEvent::Scatter) {
              if (depth++ >= params.max_depth) {
                logger.scatter_terminated();
                terminated = true;
                break;
              }
              
              // Evaluate phase function and compute new ray
              Eigen::Vector3f new_dir = sample_henyey_greenstein(r.direction(), { rng.uniform<float>(), rng.uniform<float>() }, vol.params().henyey_greenstein_g);
              r = Ray(props->point, new_dir);

              logger.scatter(r);
              scattered = true;
              break;
            } else if (event == ScatterEvent::Absorption) {
              float temp_K = props->temperature * vol.params().temperature_scale + vol.params().temperature_offset;

              if (temp_K > 400.0f) {
                // s l o w
                L += vol.params().le_scale * spectrum_to_xyz(BlackbodyEmittedRadianceSpectrum(temp_K));
              }
              logger.absorbed();
              terminated = true;
              break;
            }

            std::unreachable();
          }

          if (not scattered)
            break;
        }

        if (not terminated) {
          L += params.ambient_light * params.ambient_light_multiplier;
        }
        
        // add to the film
        m_film.data()(pt.y(), pt.x()).w() += 1.0f;
        m_film.data()(pt.y(), pt.x()).topRows<3>() += camera.params().imaging_ratio * L;
      }
    }
  }
}

} // namespace vpt