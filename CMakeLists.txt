cmake_minimum_required(VERSION 3.4...3.28 FATAL_ERROR)
project(
  WGPU_LAB
  VERSION 0.1.0
  LANGUAGES CXX C
)
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


# sample executables -------------------------------------------------------------
file(GLOB SAMPLE_FILES samples/*.cpp)
set(SAMPLE_TARGETS)
foreach(SAMPLE_FILE IN LISTS SAMPLE_FILES)
  get_filename_component(SAMPLE_FILENAME ${SAMPLE_FILE} NAME_WE)
  list(APPEND SAMPLE_TARGETS ${SAMPLE_FILENAME})
  add_executable(${SAMPLE_FILENAME}
    samples/${SAMPLE_FILENAME}.cpp
  )
  target_link_libraries(${SAMPLE_FILENAME} wgpu-lab)
endforeach(SAMPLE_FILE)


# copy shader files to binary dir ----------------------------------------------
file(GLOB SHADER_FILES shaders/*.wgsl)
foreach(SHADER_FILE IN LISTS SHADER_FILES)
  get_filename_component(SHADER_FILENAME ${SHADER_FILE} NAME)
  configure_file(${SHADER_FILE}
    ${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_FILENAME} COPYONLY)
endforeach(SHADER_FILE)


# tutorial ---------------------------------------------------------------------
add_executable(tutorial
  tutorial/webgpu-utils.h
  tutorial/webgpu-utils.cpp
  tutorial/main.cpp
)
target_link_libraries(tutorial glfw webgpu glfw3webgpu)


# set properties and compile options for all targets ---------------------------
set(ALL_TARGETS
  wgpu-lab tutorial ${SAMPLE_TARGETS}
)
foreach(LAB_TARGET IN LISTS ALL_TARGETS)
  set_target_properties(${LAB_TARGET} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR OFF
  )
  if (MSVC)
    target_compile_options(${LAB_TARGET} PRIVATE /W4)
  else()
    target_compile_options(${LAB_TARGET} PRIVATE -Wall -Wextra -pedantic)
  endif()
endforeach(LAB_TARGET)
