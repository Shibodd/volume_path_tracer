#include <vpt/volume_grid.hpp>
#include <vpt/configuration.hpp>
#include <vpt/image.hpp>
#include <vpt/worker.hpp>

#include <vpt/logging.hpp>

#include <raylib.h>


int main() {

  vpt::Configuration cfg = vpt::read_configuration("configuration.json");

  vpt::VolumeGrids vol = vpt::VolumeGrids::read_from_file(cfg.volume_path);

  vpt::TileProvider provider(cfg.output_image.size, cfg.num_waves, cfg.tile_size);

  std::vector<std::jthread> threads;

  vpt::Image<float, 3> film(cfg.output_image.size);
  film.data().fill(decltype(film)::value_t::Zero());

  vpt::Image<unsigned char, 3> img(cfg.output_image.size);

  vpt::Camera camera(cfg.camera_parameters, cfg.output_image.size);

  for (int i = 0; i < cfg.num_workers; ++i) {
    threads.emplace_back([&]() {
      vpt::RandomNumberGenerator rng(10);
      vpt::run(camera, provider, film, rng);
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
        
        for (Eigen::Index i = 0; i < img.data().rows(); ++i) {
          for (Eigen::Index j = 0; j < img.data().cols(); ++j) {
            img.data()(i,j) = (film.data()(i,j) * 255.0f).cast<unsigned char>();
          } 
        }

        UpdateTexture(texture, img.data().data());

        DrawTexture(texture, 0, 0, WHITE);
    EndDrawing();

  }

  UnloadTexture(texture);
  for (Eigen::Index i = 0; i < img.data().rows(); ++i) {
    for (Eigen::Index j = 0; j < img.data().cols(); ++j) {
      img.data()(i,j) = (film.data()(i,j) * 255.0f).cast<unsigned char>();
    } 
  }
  img.save(cfg.output_image.path.c_str());

  return 0;
}