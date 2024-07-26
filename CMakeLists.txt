cmake_minimum_required(VERSION 3.4...3.28 FATAL_ERROR)
project(
  WGPU_LAB
  VERSION 0.1.0
  LANGUAGES CXX C
)
# dependencies
add_subdirectory(libs/glfw-3.4)
add_subdirectory(libs/webgpu-dawn)
add_subdirectory(libs/glfw3webgpu)

# wgpu-lab library -------------------------------------------------------------
add_library(wgpu-lab
  src/lab_pipeline.cpp
  src/lab_shader.cpp
  src/lab_surface.cpp
  src/lab_window.cpp
  src/lab_webgpu.cpp
  src/lab.cpp

  src/headers/lab_objects.h
  src/headers/lab_pipeline.h
  src/headers/lab_shader.h
  src/headers/lab_state.h
  src/headers/lab_surface.h
  src/headers/lab_webgpu.h
  src/headers/lab_window.h
  src/headers/lab.h
)
target_include_directories(wgpu-lab PUBLIC src/headers)
target_link_libraries(wgpu-lab glfw webgpu glfw3webgpu)

set_target_properties(wgpu-lab PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
	target_compile_options(wgpu-lab PRIVATE /W4)
else()
	target_compile_options(wgpu-lab PRIVATE -Wall -Wextra -pedantic)
endif()

# test executable --------------------------------------------------------------
add_executable(test
  tests/test_window.cpp
)
target_include_directories(wgpu-lab PUBLIC src/headers)
target_link_libraries(test wgpu-lab)

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

# tutorial ---------------------------------------------------------------------
add_executable(tutorial
  tutorial/webgpu-utils.h
  tutorial/webgpu-utils.cpp
  tutorial/main.cpp
)
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
