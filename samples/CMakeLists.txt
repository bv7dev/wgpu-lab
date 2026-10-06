# libraries that only the samples use ------------------------------------------
# Both are used header-only, so they are only downloaded: SOURCE_SUBDIR points at a
# directory without a CMakeLists.txt, which keeps their own build scripts out of ours.
FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG 1.0.1
  GIT_SHALLOW ON
  SOURCE_SUBDIR headers-only)
FetchContent_Declare(tinygltf
  GIT_REPOSITORY https://github.com/syoyo/tinygltf
  GIT_TAG v2.9.3
  GIT_SHALLOW ON
  SOURCE_SUBDIR headers-only)
FetchContent_MakeAvailable(glm tinygltf)

add_library(lab-sample-libs INTERFACE)
target_include_directories(lab-sample-libs SYSTEM INTERFACE ${glm_SOURCE_DIR} ${tinygltf_SOURCE_DIR})

# one executable per .cpp file in this directory --------------------------------
# (add your own .cpp here and re-run the CMake configure step)
file(GLOB SAMPLE_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SAMPLE_NAMES)
foreach(SAMPLE_FILE IN LISTS SAMPLE_FILES)
  get_filename_component(SAMPLE_NAME ${SAMPLE_FILE} NAME_WE)
  if(EMSCRIPTEN AND SAMPLE_NAME STREQUAL "09_multi_window")
    continue() # a browser page has one canvas
  endif()
  list(APPEND SAMPLE_NAMES ${SAMPLE_NAME})
  add_executable(${SAMPLE_NAME} ${SAMPLE_FILE})
  target_link_libraries(${SAMPLE_NAME} PRIVATE lab::lab lab-sample-libs)
  # executables go to the top of the build directory, next to shaders/ and assets/
  set_target_properties(${SAMPLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  lab_target_defaults(${SAMPLE_NAME})

  if(EMSCRIPTEN)
    # a web page per sample, in site/, with the shaders and assets packed into it
    set_target_properties(${SAMPLE_NAME} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/site
      SUFFIX .html)
    target_link_options(${SAMPLE_NAME} PRIVATE
      # the blocking calls of the lab (waiting for the adapter, reading buffers back) and
      # the `while (lab::tick())` loop need the program to be able to pause for the browser
      -sASYNCIFY=1
      -sASYNCIFY_STACK_SIZE=65536
      -sALLOW_MEMORY_GROWTH=1
      "--shell-file=${CMAKE_SOURCE_DIR}/web/shell.html"
      "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/shaders@/shaders"
      "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/assets/house.glb@/assets/house.glb"
      "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/assets/ground.png@/assets/ground.png")
    set_property(TARGET ${SAMPLE_NAME} APPEND PROPERTY LINK_DEPENDS ${CMAKE_SOURCE_DIR}/web/shell.html)
  endif()

  # Every sample doubles as a smoke test: it has to run 60 frames without reporting an error.
  # These tests open windows, so they carry the label "display": `ctest -LE display` skips them.
  if(LAB_BUILD_TESTS)
    add_test(NAME sample.${SAMPLE_NAME} COMMAND ${SAMPLE_NAME} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    set_tests_properties(sample.${SAMPLE_NAME} PROPERTIES
      LABELS display
      ENVIRONMENT LAB_EXIT_AFTER_FRAMES=60
      FAIL_REGULAR_EXPRESSION "lab \\[(warn|error)\\]"
      TIMEOUT 30
      RUN_SERIAL ON)
  endif()
endforeach()

# the gallery page that links to all samples (web only) ---------------------------
if(EMSCRIPTEN)
  set(LAB_GALLERY_ITEMS "")
  foreach(SAMPLE_NAME IN LISTS SAMPLE_NAMES)
    set(IMAGE "${CMAKE_SOURCE_DIR}/docs/images/${SAMPLE_NAME}.png")
    if(EXISTS ${IMAGE})
      configure_file(${IMAGE} ${CMAKE_BINARY_DIR}/site/images/${SAMPLE_NAME}.png COPYONLY)
      set(PICTURE "<img src=\"images/${SAMPLE_NAME}.png\" alt=\"\">")
    else()
      set(PICTURE "<div class=\"blank\"></div>")
    endif()
    string(APPEND LAB_GALLERY_ITEMS "      <a href=\"${SAMPLE_NAME}.html\">${PICTURE}<span>${SAMPLE_NAME}</span></a>\n")
  endforeach()
  configure_file(${CMAKE_SOURCE_DIR}/web/index.html.in ${CMAKE_BINARY_DIR}/site/index.html @ONLY)
endif()

# copy shader and asset files to the build directory ----------------------------
# (a sample looks for its files in the working directory first, then next to its executable)
file(GLOB SHADER_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.wgsl)
foreach(SHADER_FILE IN LISTS SHADER_FILES)
  get_filename_component(SHADER_FILENAME ${SHADER_FILE} NAME)
  configure_file(${SHADER_FILE} ${CMAKE_BINARY_DIR}/shaders/${SHADER_FILENAME} COPYONLY)
endforeach()

file(GLOB ASSET_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/*)
foreach(ASSET_FILE IN LISTS ASSET_FILES)
  get_filename_component(ASSET_FILENAME ${ASSET_FILE} NAME)
  configure_file(${ASSET_FILE} ${CMAKE_BINARY_DIR}/assets/${ASSET_FILENAME} COPYONLY)
endforeach()
