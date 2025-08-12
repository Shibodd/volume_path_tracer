#include <vpt/volume_grid.hpp>
#include <vpt/configuration.hpp>
#include <vpt/image.hpp>
#include <vpt/worker.hpp>

#include <vpt/logging.hpp>

#include <raylib.h>
#include <vpt/color.hpp>
#include <vpt/spectral.hpp>

#include <random>

void film_to_image(const vpt::Image<float, 3>& film, vpt::Image<unsigned char, 3>& image) {
  for (Eigen::Index i = 0; i < image.data().rows(); ++i) {
    for (Eigen::Index j = 0; j < image.data().cols(); ++j) {
      Eigen::Vector3f xyz = film.data()(i,j);
      Eigen::Vector3f linsrgb = vpt::xyz_to_linsrgb(xyz);
      Eigen::Vector3f srgb = vpt::linsrgb_to_srgb(linsrgb);

      image.data()(i,j) = (srgb.cwiseMax(0.0f).cwiseMin(1.0f) * 255.0f).cast<unsigned char>();
    } 
  }
}

int main() {
  vpt::Configuration cfg = vpt::read_configuration("configuration.json");

  vpt::VolumeGrids vol = vpt::VolumeGrids::read_from_file(cfg.volume_path);

  vpt::TileProvider provider(cfg.output_image.size, cfg.num_waves, cfg.tile_size);

  std::vector<std::jthread> threads;

  vpt::Image<float, 3> film(cfg.output_image.size);
  film.data().fill(decltype(film)::value_t::Zero());

  vpt::Image<unsigned char, 3> img(cfg.output_image.size);

  vpt::Camera camera(cfg.camera_parameters, cfg.output_image.size);

  std::vector<vpt::Star> stars;
  stars.reserve(cfg.demo_parameters.num_stars);

  std::mt19937 gen(0);

  for (size_t i = 0; i < cfg.demo_parameters.num_stars; ++i) {

    Eigen::Vector2f screen2 = Eigen::Vector2f::Random();
    Eigen::Vector3f screen = Eigen::Vector3f { screen2.x(), screen2.y(), 0.0f };
  
    Eigen::Vector3f dir = (camera.screen_to_world_dir() * screen).normalized();

    float distance = 4.0f + std::clamp<float>(std::normal_distribution<float>(cfg.demo_parameters.distance_gauss.x(), cfg.demo_parameters.distance_gauss.y())(gen), 2.0f, 1000.0f);
    float radius = std::clamp<float>(std::normal_distribution<float>(cfg.demo_parameters.radius_gauss.x(), cfg.demo_parameters.radius_gauss.y())(gen), 0.001f, 0.03f);
    float temperature = std::clamp<float>(std::normal_distribution<float>(cfg.demo_parameters.temperature_gauss.x(), cfg.demo_parameters.temperature_gauss.y())(gen), 3000.0f, 40000.0f);

    stars.emplace_back(
      cfg.camera_parameters.position + dir * distance,
      radius,
      vpt::spectrum_to_xyz(vpt::BlackbodyEmittedRadianceSpectrum(temperature))
    );
  }

  for (int i = 0; i < cfg.num_workers; ++i) {
    threads.emplace_back([&]() {
      vpt::RandomNumberGenerator rng(10);
      vpt::run(stars, camera, provider, film, rng);
      vptINFO(std::this_thread::get_id() << " IS DONE!");
    });
  }

  InitWindow(cfg.output_image.size.x(), cfg.output_image.size.y(), "raylib [core] example - basic window");
  SetTargetFPS(30);

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
        ClearBackground(RAYWHITE);
        
        film_to_image(film, img);

        UpdateTexture(texture, img.data().data());

        DrawTexture(texture, 0, 0, WHITE);
    EndDrawing();

  }

  UnloadTexture(texture);
  
  film_to_image(film, img);
  img.save(cfg.output_image.path.c_str());

  return 0;
}