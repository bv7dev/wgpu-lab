cmake_minimum_required(VERSION 3.13...3.24 FATAL_ERROR)
project(
  wgpu-lab
  VERSION 0.1.0
  LANGUAGES CXX C)

# check if python is installed and call script to fetch lab's dependencies -------
find_package(Python3 REQUIRED)
message(STATUS "wgpu-lab: using python at ${Python3_EXECUTABLE}")
message(STATUS "Running lab_fetch_dependencies.py:")
execute_process(COMMAND
  ${Python3_EXECUTABLE}
  "${PROJECT_SOURCE_DIR}/scripts/lab_fetch_dependencies.py"
  --root ${PROJECT_SOURCE_DIR})

# dawn config ------------------------------------------------------------------
set(DAWN_FETCH_DEPENDENCIES ON)
set(DAWN_BUILD_SAMPLES OFF)

set(TINT_BUILD_SPV_READER OFF)
set(TINT_BUILD_CMD_TOOLS OFF)
set(TINT_BUILD_TESTS OFF)
set(TINT_BUILD_IR_BINARY OFF)

add_subdirectory(libs/dawn)

# glm config -------------------------------------------------------------------
set(GLM_ENABLE_CXX_20 ON CACHE INTERNAL "" FORCE)

add_subdirectory(libs/glm)

# tinygltf config --------------------------------------------------------------
set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)

add_subdirectory(libs/tinygltf)

# wgpu-lab library -------------------------------------------------------------
add_library(wgpu-lab
  src/detail/lab_pipeline.cpp
  src/detail/lab_shader.cpp
  src/detail/lab_surface.cpp
  src/detail/lab_webgpu.cpp
  src/detail/lab_window.cpp
  src/detail/lab.cpp

  src/extra/lab_enums.h
  src/extra/lab_mapped_vram.h
  src/extra/lab_objects.h
  src/extra/lab_public.h
  src/extra/lab_state.h

  src/objects/lab_buffer.h
  src/objects/lab_pipeline.h
  src/objects/lab_shader.h
  src/objects/lab_surface.h
  src/objects/lab_webgpu.h
  src/objects/lab_window.h

  src/lab)

target_include_directories(wgpu-lab PUBLIC src)
target_link_libraries(wgpu-lab PUBLIC
  webgpu_cpp
  dawn_internal_config
  dawn::dawn_common
  dawn::dawn_proc
  dawn::dawn_glfw
  dawn::dawn_native
  glfw)

# sample executables -----------------------------------------------------------
file(GLOB SAMPLE_FILES samples/*.cpp)
set(SAMPLE_TARGETS)
foreach(SAMPLE_FILE IN LISTS SAMPLE_FILES)
  get_filename_component(SAMPLE_FILENAME ${SAMPLE_FILE} NAME_WE)
  list(APPEND SAMPLE_TARGETS ${SAMPLE_FILENAME})
  add_executable(${SAMPLE_FILENAME}
    samples/${SAMPLE_FILENAME}.cpp)
  target_include_directories(${SAMPLE_FILENAME} PRIVATE libs/tinygltf src samples)
  target_link_libraries(${SAMPLE_FILENAME} PRIVATE wgpu-lab glm::glm)
endforeach(SAMPLE_FILE)

# copy shader files to binary dir ----------------------------------------------
file(GLOB SHADER_FILES shaders/*.wgsl)
foreach(SHADER_FILE IN LISTS SHADER_FILES)
  get_filename_component(SHADER_FILENAME ${SHADER_FILE} NAME)
  configure_file(${SHADER_FILE}
    ${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_FILENAME} COPYONLY)
endforeach(SHADER_FILE)

# copy asset files to binary dir -----------------------------------------------
file(GLOB ASSET_FILES assets/**)
foreach(ASSET_FILE IN LISTS ASSET_FILES)
  get_filename_component(ASSET_FILENAME ${ASSET_FILE} NAME)
  configure_file(${ASSET_FILE}
    ${CMAKE_CURRENT_BINARY_DIR}/assets/${ASSET_FILENAME} COPYONLY)
endforeach(ASSET_FILE)

# set properties and compile options for all targets ---------------------------
set(ALL_TARGETS
  wgpu-lab ${SAMPLE_TARGETS})
foreach(LAB_TARGET IN LISTS ALL_TARGETS)
  set_target_properties(${LAB_TARGET} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR OFF)
  if (MSVC)
    target_compile_options(${LAB_TARGET} PRIVATE /W3)
  else()
    target_compile_options(${LAB_TARGET} PRIVATE -Wall -Wextra -pedantic)
  endif()
endforeach(LAB_TARGET)
