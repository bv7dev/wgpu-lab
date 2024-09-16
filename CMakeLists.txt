cmake_minimum_required(VERSION 3.13)
project(
  wgpu-lab
  VERSION 0.1.0
  LANGUAGES CXX C)

set(DAWN_FETCH_DEPENDENCIES ON)
add_subdirectory(libs/dawn)

set(GLM_ENABLE_CXX_20 ON)
add_subdirectory(libs/glm)

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
target_link_libraries(wgpu-lab PUBLIC dawn::webgpu_dawn dawn::dawn_glfw glfw)

# sample executables -------------------------------------------------------------
file(GLOB SAMPLE_FILES samples/*.cpp)
set(SAMPLE_TARGETS)
foreach(SAMPLE_FILE IN LISTS SAMPLE_FILES)
  get_filename_component(SAMPLE_FILENAME ${SAMPLE_FILE} NAME_WE)
  list(APPEND SAMPLE_TARGETS ${SAMPLE_FILENAME})
  add_executable(${SAMPLE_FILENAME}
    samples/${SAMPLE_FILENAME}.cpp)
  target_include_directories(${SAMPLE_FILENAME} PRIVATE libs/tinygltf)
  target_link_libraries(${SAMPLE_FILENAME} PRIVATE wgpu-lab glm::glm)
endforeach(SAMPLE_FILE)

# copy shader files to binary dir ----------------------------------------------
file(GLOB SHADER_FILES shaders/*.wgsl)
foreach(SHADER_FILE IN LISTS SHADER_FILES)
  get_filename_component(SHADER_FILENAME ${SHADER_FILE} NAME)
  configure_file(${SHADER_FILE}
    ${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_FILENAME} COPYONLY)
endforeach(SHADER_FILE)

# copy asset files to binary dir ----------------------------------------------
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
    target_compile_options(${LAB_TARGET} PRIVATE /W4)
    target_compile_options(${LAB_TARGET} PRIVATE /wd4324)
  else()
    target_compile_options(${LAB_TARGET} PRIVATE -Wall -Wextra -pedantic)
  endif()
endforeach(LAB_TARGET)
