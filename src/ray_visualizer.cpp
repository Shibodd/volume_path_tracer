


#include "vpt/configuration.hpp"
#include "vpt/volume_grids.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-private-field"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#include "nanovdb/math/HDDA.h"
#include "nanovdb/math/Math.h"
#include "nanovdb/math/Ray.h"
#include "vpt/nanovdb_utils.hpp"
#include "vpt/utils.hpp"
#include <vpt/ray.hpp>
#include <vpt/volume.hpp>
#include <vpt/camera.hpp>
#include <raylib.h>
#pragma GCC diagnostic pop

Vector3 eigen_to_raylib_f(const Eigen::Vector3f& v) {
    return { v.x(), v.y(), v.z() };
}

Vector3 eigen_to_raylib_i(const Eigen::Vector3i& v) {
    return eigen_to_raylib_f(v.cast<float>());
}

struct Result {
    std::vector<vpt::RayMajorantIterator::DDAStep> dda_steps;
    std::vector<vpt::RayMajorantIterator::Segment> segments;
};

Result simulate(vpt::RayMajorantIterator& iter) {
    Result ans;

    iter.record_steps(&ans.dda_steps);

    while (auto seg = iter.next()) {
        ans.segments.push_back(*seg);
    }

    return ans;
}

void draw(const Result& result, const vpt::VolumeGrids::AccessorT& density, const nanovdb::math::Ray<float>& ray);


int main() {
    vpt::Configuration cfg = vpt::read_configuration("scenes/fire.json");

    vpt::VolumeGrids grids = vpt::VolumeGrids::read_from_file("volumes/fire.nvdb");
    vpt::Volume vol(grids, cfg.volume_parameters);

    vpt::Camera camera(cfg.camera_parameters, cfg.output_size);

    vpt::Ray vpt_ray = camera.generate_ray(cfg.worker_parameters.single_pixel.coord, Eigen::Vector2f::Zero());

    auto density_acc = vol.grids().density().getAccessor();

    auto intr = vol.intersect(vpt_ray, density_acc);

    auto result = simulate(*intr);

    draw(result, density_acc, intr->ray());
}

void draw(const Result& result, const vpt::VolumeGrids::AccessorT& density, const nanovdb::math::Ray<float>& ray) {
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera free");
    ToggleFullscreen();


    auto ray_start = nanovdb_to_eigen_f(ray.eye()) + nanovdb_to_eigen_f(ray.dir()) * ray.t0();
    auto ray_end = nanovdb_to_eigen_f(ray.eye()) + nanovdb_to_eigen_f(ray.dir()) * ray.t1();

    Eigen::Vector3f v2;
    Eigen::Vector3f v3;

    vpt::coordinate_system((ray_end - ray_start).normalized(), v2, v3);
    

    // Define the camera to look into our 3d world
    Camera3D camera;
    camera.position = eigen_to_raylib_f(ray_start);
    camera.target = eigen_to_raylib_f(ray_end);
    camera.up = Vector3 { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FREE);

        if (IsKeyPressed('Z')) camera.target = Vector3 { 0.0f, 0.0f, 0.0f };
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                for (auto step : result.dda_steps) {
                    DrawSphere(eigen_to_raylib_f(step.hit), 0.1f, LIGHTGRAY);
                    DrawCubeWires(eigen_to_raylib_f(step.voxel.cast<float>() + Eigen::Vector3f::Constant(0.5f * step.dim)), step.dim, step.dim, step.dim, LIGHTGRAY);
                }

                for (auto seg : result.segments) {
                    auto seg_start = nanovdb_to_eigen_f(ray(seg.t0));
                    auto seg_end = nanovdb_to_eigen_f(ray(seg.t1));

                    auto off = v2 * 0.5f;

                    DrawLine3D(eigen_to_raylib_f(seg_start + off), eigen_to_raylib_f(seg_end + off), GREEN);
                    DrawSphere(eigen_to_raylib_f(seg_start + off), 0.1f, GREEN);
                    DrawSphere(eigen_to_raylib_f(seg_end + off), 0.1f, GREEN);
                }
                
                DrawSphere(eigen_to_raylib_f(ray_start), 0.1f, BLUE);
                
                DrawLine3D(eigen_to_raylib_f(ray_start), eigen_to_raylib_f(ray_end), RED);
                DrawGrid(100, 8.0f);

            EndMode3D();

            for (auto it = result.dda_steps.cbegin(); it != result.dda_steps.cend(); ++it) {
                const auto& step = *it;

                Eigen::Vector3f center = step.voxel.cast<float>() + Eigen::Vector3f::Constant(0.5f * step.dim);
                
                size_t i = std::distance(result.dda_steps.cbegin(), it);

                std::ostringstream ss;
                ss << "Step" << i << ": " << step.majorant;
                std::string txt = ss.str();

                int width = MeasureText(txt.c_str(), 20);
                Vector2 screen = GetWorldToScreen(eigen_to_raylib_f(center), camera);
                DrawText(txt.c_str(), static_cast<int>(screen.x) - width / 2, static_cast<int>(screen.y), 20, BLACK);
            }

            for (auto it = result.segments.cbegin(); it != result.segments.cend(); ++it) {
                const auto& seg = *it;

                auto seg_start = nanovdb_to_eigen_f(ray(seg.t0));
                auto seg_end = nanovdb_to_eigen_f(ray(seg.t1));

                size_t i = std::distance(result.segments.cbegin(), it);

                std::ostringstream ss;
                ss << "Seg" << i << ": " << seg.d_maj;

                std::string txt = ss.str();
                int width = MeasureText(txt.c_str(), 20);
                Vector2 screen = GetWorldToScreen(eigen_to_raylib_f((seg_end + seg_start) / 2), camera);
                DrawText(txt.c_str(), static_cast<int>(screen.x) - width / 2, static_cast<int>(screen.y), 20, BLACK);
            }

            DrawRectangle( 10, 10, 320, 93, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines( 10, 10, 320, 93, BLUE);

            DrawText("Free camera default controls:", 20, 20, 10, BLACK);
            DrawText("- Mouse Wheel to Zoom in-out", 40, 40, 10, DARKGRAY);
            DrawText("- Mouse Wheel Pressed to Pan", 40, 60, 10, DARKGRAY);
            DrawText("- Z to zoom to (0, 0, 0)", 40, 80, 10, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}