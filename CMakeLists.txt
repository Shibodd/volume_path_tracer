cmake_minimum_required (VERSION 3.18)
project (volume_path_tracer)

add_executable (${PROJECT_NAME} main.cpp)
target_include_directories (${PROJECT_NAME} PRIVATE include)
