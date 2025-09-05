#include <vpt/volume.hpp>
#include <vpt/configuration.hpp>
#include <vpt/image.hpp>
#include <vpt/worker.hpp>

#include <vpt/logging.hpp>

#include <raylib.h>
#include <vpt/color.hpp>
#include <vpt/spectral.hpp>

void film_to_image(const vpt::Image<float, 4>& film, vpt::Image<unsigned char, 3>& image) {
  for (Eigen::Index i = 0; i < image.data().rows(); ++i) {
    for (Eigen::Index j = 0; j < image.data().cols(); ++j) {
      Eigen::Vector4f xyzw = film.data()(i,j);
      Eigen::Vector3f xyz = xyzw.topRows<3>() / xyzw.w();

      Eigen::Vector3f linsrgb = vpt::xyz_to_linsrgb(xyz);
      Eigen::Vector3f srgb = vpt::linsrgb_to_srgb(linsrgb);

      image.data()(i,j) = (srgb.cwiseMax(0.0f).cwiseMin(1.0f) * 255.0f).cast<unsigned char>();
    } 
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    vptFATAL("Usage: " << argv[0] << " config_path output_path");
    return 1;
  }

  vpt::init_blackbody_radiation_xyz();

  std::filesystem::path config_path = std::filesystem::canonical(argv[1]);
  std::filesystem::path output_path(argv[2]);

  vpt::Configuration cfg = vpt::read_configuration(config_path);

  // vpt::VolumeGrids grids = vpt::VolumeGrids::generate_donut();
  vpt::VolumeGrids grids = vpt::VolumeGrids::read_from_file(config_path.parent_path() / cfg.volume_path);
  vpt::Volume vol(grids, cfg.volume_parameters);

  if (grids.has_temperature())
    std::cout << "TempMin: " << grids.temperature().tree().root().minimum() << ", TempMax: " << grids.temperature().tree().root().maximum() << std::endl;

  vpt::TileProvider provider(cfg.output_size, cfg.num_waves, cfg.tile_size);

  std::vector<std::jthread> threads;

  vpt::Image<float, 4> film(cfg.output_size);
  film.data().fill(decltype(film)::value_t::Zero());

  vpt::Image<unsigned char, 3> img(cfg.output_size);
  img.data().fill(decltype(img)::value_t::Zero());

  vpt::Camera camera(cfg.camera_parameters, cfg.output_size);

  std::mutex completion_mtx;
  unsigned int completion_count = 0;
  std::chrono::milliseconds completion_max_elapsed(0);

  provider.reset_eta();
  for (unsigned int i = 0; i < cfg.num_workers; ++i) {
    threads.emplace_back([&]() {
      auto start = std::chrono::high_resolution_clock::now();

      vpt::RandomNumberGenerator rng(cfg.seed);
      vpt::run(cfg.worker_parameters, vol, camera, provider, film, rng);

      auto end = std::chrono::high_resolution_clock::now();

      auto elapsed = std::chrono::duration_cast<decltype(completion_max_elapsed)>(end - start);

      {
        std::lock_guard lock(completion_mtx);
        ++completion_count;
        completion_max_elapsed = std::max(elapsed, completion_max_elapsed);
      }

      vptINFO(std::this_thread::get_id() << " IS DONE!");

      if (completion_count == cfg.num_workers) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300)); // poor man's synchronization :)
        vptINFO("Rendering complete in " << completion_max_elapsed.count() << " ms");
      }
    });
  }

  InitWindow(cfg.output_size.x(), cfg.output_size.y(), ("vpt - " + cfg.volume_path.filename().string()).c_str());
  SetTargetFPS(5);

  Image image;
  image.data = img.data().data();
  image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
  image.width = cfg.output_size.x();
  image.height = cfg.output_size.y();
  image.mipmaps = 1;

  Texture2D texture = LoadTextureFromImage(image);

  while (!WindowShouldClose())
  { 
    BeginDrawing();
        ClearBackground(PURPLE);
        
        film_to_image(film, img);

        UpdateTexture(texture, img.data().data());

        DrawTexture(texture, 0, 0, WHITE);

        unsigned int prog = provider.progress();
        auto eta = std::chrono::duration_cast<std::chrono::seconds>(provider.eta());

        auto eta_mm = std::chrono::duration_cast<std::chrono::minutes>(eta);
        auto eta_ss = eta % 60;

        std::ostringstream oss;
        oss << prog << '%' << " - ETA: " << eta_mm.count() << "m " << eta_ss.count() << "s";

        auto pixel = img.data()(20, 20);
        float luminance = (0.299 * pixel.x() + 0.587 * pixel.y() + 0.114 * pixel.z())/255;
        
        Color color;
        if (luminance > 0.5f)
          color = BLACK;
        else
          color = WHITE;

        DrawText(oss.str().c_str(), 20, 20, 24, color);
    EndDrawing();
  }

  UnloadTexture(texture);

  vptINFO("Waiting for all workers to reach the next wave before saving the image...");
  provider.stop_at_next_wave();
  for (auto& thr : threads) {
    thr.join();
  }
  
  film_to_image(film, img);
  img.save(output_path.c_str());

  return 0;
}