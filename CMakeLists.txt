cmake_minimum_required(VERSION 3.28)
project(
  wgpu-lab
  VERSION 0.3.0
  LANGUAGES CXX C)

# options ----------------------------------------------------------------------
# samples and tests are only built when wgpu-lab is the top-level project, so that pulling
# the lab into another project (add_subdirectory / FetchContent) yields just the library
option(LAB_BUILD_SAMPLES "Build the sample executables" ${PROJECT_IS_TOP_LEVEL})
option(LAB_BUILD_TESTS "Build the tests (run them with ctest)" ${PROJECT_IS_TOP_LEVEL})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(FetchContent)

# the lab uses headers, not C++ modules: skip the per-file module dependency scan
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "wgpu-lab: using ccache at ${CCACHE_PROGRAM}")
  endif()
endif()

# dawn -------------------------------------------------------------------------
include(LabDawn)

# glfw: an installed 3.4+ is used if there is one, otherwise it is fetched ------
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_INSTALL OFF)
FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG 3.4
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
  SYSTEM
  FIND_PACKAGE_ARGS 3.4 NAMES glfw3)
FetchContent_MakeAvailable(glfw)

# compile options shared by the library and its samples -------------------------
function(lab_target_defaults LAB_TARGET)
  set_target_properties(${LAB_TARGET} PROPERTIES
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR OFF)
  if(MSVC)
    target_compile_options(${LAB_TARGET} PRIVATE /W3)
  else()
    target_compile_options(${LAB_TARGET} PRIVATE -Wall -Wextra -pedantic)
  endif()
endfunction()

# wgpu-lab library -------------------------------------------------------------
add_library(wgpu-lab
  src/lab_buffer.cpp
  src/lab_frame.cpp
  src/lab_gpu.cpp
  src/lab_image.cpp
  src/lab_pipeline.cpp
  src/lab_runtime.cpp
  src/lab_shader.cpp
  src/lab_surface.cpp
  src/lab_texture.cpp
  src/lab_window.cpp
  src/lab_detail.h

  src/platform/lab_paths.cpp
  src/platform/lab_surface_glfw.cpp
  src/platform/lab_platform.h

  include/lab
  include/lab_buffer.h
  include/lab_error.h
  include/lab_frame.h
  include/lab_gpu.h
  include/lab_input.h
  include/lab_label.h
  include/lab_log.h
  include/lab_pipeline.h
  include/lab_shader.h
  include/lab_surface.h
  include/lab_texture.h
  include/lab_window.h)
add_library(lab::lab ALIAS wgpu-lab)

# include/ is the public interface, src/ is private
target_include_directories(wgpu-lab PUBLIC include PRIVATE src)
target_compile_features(wgpu-lab PUBLIC cxx_std_23)
target_link_libraries(wgpu-lab PUBLIC dawn::webgpu_dawn glfw)
lab_target_defaults(wgpu-lab)

# which window systems the surface glue is compiled for (Linux only)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  option(LAB_USE_WAYLAND "Support Wayland windows (needs a GLFW built with Wayland)" ON)
  option(LAB_USE_X11 "Support X11 windows (needs a GLFW built with X11)" ON)
  if(LAB_USE_WAYLAND)
    target_compile_definitions(wgpu-lab PRIVATE LAB_USE_WAYLAND)
  endif()
  if(LAB_USE_X11)
    target_compile_definitions(wgpu-lab PRIVATE LAB_USE_X11)
  endif()
endif()

# tests and sample executables --------------------------------------------------
if(LAB_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(LAB_BUILD_SAMPLES)
  add_subdirectory(samples)
endif()
