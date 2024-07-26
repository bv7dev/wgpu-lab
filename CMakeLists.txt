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


# test executables -------------------------------------------------------------
file(GLOB TEST_FILES tests/*.cpp)
set(TEST_TARGETS)
foreach(TEST_FILE IN LISTS TEST_FILES)
  get_filename_component(TEST_FILENAME ${TEST_FILE} NAME_WE)
  list(APPEND TEST_TARGETS ${TEST_FILENAME})
  add_executable(${TEST_FILENAME}
    tests/${TEST_FILENAME}.cpp
  )
  target_link_libraries(${TEST_FILENAME} wgpu-lab)
endforeach(TEST_FILE)


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
  wgpu-lab tutorial ${TEST_TARGETS}
)
foreach(LAB_TARGET IN LISTS ALL_TARGETS)
  set_target_properties(${LAB_TARGET} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
  )
  if (MSVC)
    target_compile_options(${LAB_TARGET} PRIVATE /W4)
  else()
    target_compile_options(${LAB_TARGET} PRIVATE -Wall -Wextra -pedantic)
  endif()
endforeach(LAB_TARGET)
