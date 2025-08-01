cmake_minimum_required (VERSION 3.18)
project (vpt)

include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        40626af88bd7df9a5fb80be7b25ac85b122d6c21)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glaze)

add_subdirectory (external)

add_executable (${PROJECT_NAME}
  src/main.cpp
  src/vdb_grid_io.cpp
  src/configuration.cpp
)
target_include_directories (${PROJECT_NAME} PRIVATE include)
target_link_libraries (${PROJECT_NAME} cimg nanovdb fmt::fmt glaze::glaze)
target_compile_features (${PROJECT_NAME} PRIVATE cxx_std_23)