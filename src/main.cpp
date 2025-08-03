#include <vpt/volume_grid.hpp>
#include <vpt/configuration.hpp>
#include <vpt/image.hpp>
#include <vpt/worker.hpp>

#include <random>

#include <vpt/logging.hpp>

#include <raylib.h>

int main() {
  vpt::Configuration cfg = vpt::read_configuration("configuration.json");

  vpt::VolumeGrids vol = vpt::VolumeGrids::read_from_file(cfg.volume_path);

  vpt::TileProvider provider(cfg.output_image.size, cfg.num_waves, cfg.tile_size);

  std::vector<std::jthread> threads;

  vpt::Image<unsigned char, 3> img(cfg.output_image.size);
  img.data().fill(decltype(img)::value_t(0, 0, 0));

  

  for (int i = 0; i < cfg.num_workers; ++i) {
    threads.emplace_back([&]() {
      std::mt19937 gen(101 + i);
      std::normal_distribution<> dis(0.0, 5);

      while (auto handle = provider.next()) {
        std::this_thread::sleep_for(std::chrono::duration<double>(std::abs(dis(gen))));

        vpt::image_rect_t rect = handle.compute_rect();

        img
          .data()
          .block(rect.start.y(), rect.start.x(), rect.size.y(), rect.size.x())
          .array() += decltype(img)::value_t(50, 50, 50);

        vptINFO(std::this_thread::get_id() << ": " << rect);
      }

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
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      auto pos = GetMousePosition();

      if (pos.x >= 0 && pos.y >= 0 && pos.x < img.data().cols() && pos.y < img.data().rows()) {
        size_t x = std::min<size_t>(pos.x, img.data().cols());
        size_t y = std::min<size_t>(pos.y, img.data().rows());

        img.data()(y, x) = decltype(img)::value_t::Ones() * 255;
      }
    }
    
    BeginDrawing();
        ClearBackground(RAYWHITE);
        UpdateTexture(texture, img.data().data());

        DrawTexture(texture, 0, 0, WHITE);
    EndDrawing();

  }

  UnloadTexture(texture);
  img.save(cfg.output_image.path.c_str());

  return 0;
}