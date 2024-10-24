cmake_minimum_required(VERSION 3.0.0...3.24 FATAL_ERROR)
project(webgpu-backend-dawn VERSION 1.0.0)

message(STATUS "Using Dawn backend for WebGPU")

add_library(webgpu INTERFACE)


include(cmake/FetchDawn.cmake)

target_link_libraries(webgpu INTERFACE webgpu_dawn)
target_include_directories(webgpu INTERFACE
  "${CMAKE_BINARY_DIR}/_deps/dawn-src/include"
)

# This is used to advertise the flavor of WebGPU that this zip provides
target_compile_definitions(webgpu INTERFACE WEBGPU_BACKEND_DAWN)
