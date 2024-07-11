cmake_minimum_required(VERSION 3.20)
project(lab VERSION 0.1.0)

add_subdirectory(libs/glfw)
add_subdirectory(libs/webgpu)

add_executable(lab src/main.cpp)
target_link_libraries(lab glfw webgpu)
