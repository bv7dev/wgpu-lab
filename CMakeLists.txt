cmake_minimum_required(VERSION 3.4...3.28 FATAL_ERROR)
project(
  wgpu-lab
  VERSION 0.1.0
  LANGUAGES CXX C
)

add_subdirectory(libs/glfw-3.4)
add_subdirectory(libs/webgpu-dawn)
add_subdirectory(libs/glfw3webgpu)

# Tutorial ---------------------------------------------------------------------
add_executable(tutorial
  tutorial/webgpu-utils.h
  tutorial/webgpu-utils.cpp
  tutorial/main.cpp
)

target_include_directories(tutorial PRIVATE src)
target_link_libraries(tutorial glfw webgpu glfw3webgpu)

set_target_properties(tutorial PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
	target_compile_options(tutorial PRIVATE /W4)
else()
	target_compile_options(tutorial PRIVATE -Wall -Wextra -pedantic)
endif()

# Test -------------------------------------------------------------------------
add_executable(test
  src/lab_state.h
  src/lab_surface.h
  src/lab_surface.cpp
  src/lab_window.h
  src/lab_window.cpp
  src/lab_webgpu.h
  src/lab_webgpu.cpp
  src/lab.h
  src/lab.cpp
  tests/test_window.cpp
)

target_include_directories(test PRIVATE src)
target_link_libraries(test glfw webgpu glfw3webgpu)

set_target_properties(test PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
	target_compile_options(test PRIVATE /W4)
else()
	target_compile_options(test PRIVATE -Wall -Wextra -pedantic)
endif()
