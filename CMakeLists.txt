cmake_minimum_required (VERSION 3.18)
project (volume_path_tracer)

add_subdirectory (external)

add_executable (${PROJECT_NAME} src/main.cpp)
target_include_directories (${PROJECT_NAME} PRIVATE include)
target_link_libraries (${PROJECT_NAME} PRIVATE cimg nanovdb)