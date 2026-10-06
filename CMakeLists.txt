cmake_minimum_required(VERSION 3.28)
project(
  wgpu-lab
  VERSION 0.5.0
  LANGUAGES CXX C)

# options ----------------------------------------------------------------------
# samples and tests are only built when wgpu-lab is the top-level project, so that pulling
# the lab into another project (add_subdirectory / FetchContent) yields just the library
option(LAB_BUILD_SAMPLES "Build the sample executables" ${PROJECT_IS_TOP_LEVEL})
if(EMSCRIPTEN)
  # the tests need a native GPU and a file system, the web gets the samples as a gallery
  set(LAB_BUILD_TESTS OFF)
else()
  option(LAB_BUILD_TESTS "Build the tests (run them with ctest)" ${PROJECT_IS_TOP_LEVEL})
endif()

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

if(EMSCRIPTEN)
  # the web (configure with `emcmake cmake --preset web`) ------------------------
  # WebGPU is the browser's own. webgpu_cpp.h comes from Dawn's Emscripten port of
  # the very same Dawn release the native build is pinned to; GLFW from the
  # contrib.glfw3 port. Both are Emscripten "ports": --use-port fetches and builds them.
  add_library(dawn::webgpu_dawn INTERFACE IMPORTED)
  target_compile_options(dawn::webgpu_dawn INTERFACE "--use-port=${CMAKE_CURRENT_SOURCE_DIR}/cmake/emdawnwebgpu.remoteport.py")
  target_link_options(dawn::webgpu_dawn INTERFACE "--use-port=${CMAKE_CURRENT_SOURCE_DIR}/cmake/emdawnwebgpu.remoteport.py")
  add_library(glfw INTERFACE IMPORTED)
  target_compile_options(glfw INTERFACE --use-port=contrib.glfw3)
  target_link_options(glfw INTERFACE --use-port=contrib.glfw3)
else()
  # dawn -------------------------------------------------------------------------
  include(LabDawn)
  if(WIN32)
    include(LabWindowsRuntime) # the DLLs Dawn loads at run time
  endif()

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
endif()

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
  if(EMSCRIPTEN)
    # exceptions are off by default on the web, the lab reports mistakes with them
    target_compile_options(${LAB_TARGET} PRIVATE -fexceptions)
    target_link_options(${LAB_TARGET} PRIVATE -fexceptions)
  endif()
  get_target_property(LAB_TARGET_TYPE ${LAB_TARGET} TYPE)
  if(WIN32 AND LAB_TARGET_TYPE STREQUAL "EXECUTABLE")
    lab_copy_runtime_dlls(${LAB_TARGET})
  endif()
endfunction()

# wgpu-lab library -------------------------------------------------------------
add_library(wgpu-lab
  src/lab_buffer.cpp
  src/lab_compute.cpp
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

  src/platform/lab_platform.h

  include/lab
  include/lab_buffer.h
  include/lab_compute.h
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

# the platform specific part: desktop window systems, or the browser
if(EMSCRIPTEN)
  target_sources(wgpu-lab PRIVATE src/platform/lab_surface_web.cpp)
else()
  target_sources(wgpu-lab PRIVATE src/platform/lab_paths.cpp src/platform/lab_surface_glfw.cpp)
endif()

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
