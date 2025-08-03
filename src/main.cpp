#include <vpt/volume_grid.hpp>
#include <vpt/configuration.hpp>
#include <vpt/image.hpp>

#include <fmt/base.h>

#include <raylib.h>

int main() {
  vpt::Configuration cfg = vpt::read_configuration("configuration.json");

  vpt::VolumeGrids vol = vpt::VolumeGrids::read_from_file(cfg.volume_path);

  vpt::Image<unsigned char, 3> img(cfg.output_image.width, cfg.output_image.height);
  img.data().fill(decltype(img)::value_t(50, 50, 50));

  img.data()(50,0) = decltype(img)::value_t(100, 150, 200);

  InitWindow(cfg.output_image.width, cfg.output_image.height, "raylib [core] example - basic window");
  SetTargetFPS(30);

  Image image;
  image.data = img.data().data();
  image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
  image.width = cfg.output_image.width;
  image.height = cfg.output_image.height;
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