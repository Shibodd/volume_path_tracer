cmake_minimum_required (VERSION 3.18)
project (vpt)

include(FetchContent)

FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glaze)

FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(raylib)

add_subdirectory (external)

find_package (Eigen3 3.3 REQUIRED NO_MODULE)

add_executable (${PROJECT_NAME}
  src/main.cpp
  src/volume_grids.cpp
  src/volume.cpp
  src/majorant_transmittance_sampler.cpp
  src/configuration.cpp
  src/image_io.cpp
  src/tile_provider.cpp
  src/camera.cpp
  src/ray.cpp
  src/worker.cpp
  src/spectral.cpp
)

target_include_directories (${PROJECT_NAME} PRIVATE include)
target_compile_options (${PROJECT_NAME} PRIVATE -Wall -Wpedantic -Werror -Wextra -Wdouble-promotion -Wfloat-conversion)
target_link_libraries (${PROJECT_NAME} nanovdb glaze::glaze Eigen3::Eigen raylib spng pcg-cpp)
target_compile_features (${PROJECT_NAME} PRIVATE cxx_std_23)