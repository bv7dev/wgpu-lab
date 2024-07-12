cmake_minimum_required(VERSION 3.4...3.28 FATAL_ERROR)
project(
  wgpu-lab
  VERSION 0.1.0
  LANGUAGES CXX C
)

add_subdirectory(libs/glfw-3.4)
add_subdirectory(libs/webgpu-dawn)
add_subdirectory(libs/glfw3webgpu)

add_executable(${CMAKE_PROJECT_NAME}
  src/webgpu-utils.cpp
  src/webgpu-utils.h
  src/main.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}
  glfw webgpu glfw3webgpu
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
	target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4)
else()
	target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()
