cmake_minimum_required(VERSION 3.4...3.28 FATAL_ERROR)
project(lab VERSION 0.1.0)

add_subdirectory(libs/glfw-3.4)
add_subdirectory(libs/webgpu-dawn)

add_executable(lab src/main.cpp)
target_link_libraries(lab glfw webgpu)
