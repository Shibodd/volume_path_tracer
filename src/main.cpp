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
  if (argc != 2) {
    vptFATAL("Usage: " << argv[0] << " config_path");
    return 1;
  }

  vpt::init_blackbody_radiation_xyz();

  vpt::Configuration cfg = vpt::read_configuration(argv[1]);

  // vpt::VolumeGrids grids = vpt::VolumeGrids::generate_donut();
  vpt::VolumeGrids grids = vpt::VolumeGrids::read_from_file(cfg.volume_path);
  vpt::Volume vol(grids, cfg.volume_parameters);

  if (grids.has_temperature())
    std::cout << "TempMin: " << grids.temperature().tree().root().minimum() << ", TempMax: " << grids.temperature().tree().root().maximum() << std::endl;

  vpt::TileProvider provider(cfg.output_image.size, cfg.num_waves, cfg.tile_size);

  std::vector<std::jthread> threads;

  vpt::Image<float, 4> film(cfg.output_image.size);
  film.data().fill(decltype(film)::value_t::Zero());

  vpt::Image<unsigned char, 3> img(cfg.output_image.size);
  img.data().fill(decltype(img)::value_t::Zero());

  vpt::Camera camera(cfg.camera_parameters, cfg.output_image.size);

  for (unsigned int i = 0; i < cfg.num_workers; ++i) {
    threads.emplace_back([&]() {
      vpt::RandomNumberGenerator rng(10);
      vpt::run(cfg.worker_parameters, vol, camera, provider, film, rng);
      vptINFO(std::this_thread::get_id() << " IS DONE!");
    });
  }

  InitWindow(cfg.output_image.size.x(), cfg.output_image.size.y(), ("vpt - " + cfg.volume_path.filename().string()).c_str());
  SetTargetFPS(5);

  Image image;
  image.data = img.data().data();
  image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
  image.width = cfg.output_image.size.x();
  image.height = cfg.output_image.size.y();
  image.mipmaps = 1;

  Texture2D texture = LoadTextureFromImage(image);

  while (!WindowShouldClose())    // Detect window close button or ESC key
  { 
    BeginDrawing();
        ClearBackground(PURPLE);
        
        film_to_image(film, img);

        UpdateTexture(texture, img.data().data());

        DrawTexture(texture, 0, 0, WHITE);

        unsigned int prog = provider.progress();

        DrawText((std::to_string(prog) + "%").c_str(), 20, 20, 24, BLACK);
    EndDrawing();
  }

  UnloadTexture(texture);

  vptINFO("Waiting for all workers to reach the next wave before saving the image...");
  provider.stop_at_next_wave();
  for (auto& thr : threads) {
    thr.join();
  }
  
  film_to_image(film, img);
  img.save(cfg.output_image.path.c_str());

  return 0;
}