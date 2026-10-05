# doctest is used header-only and therefore only downloaded,
# see samples/CMakeLists.txt for the SOURCE_SUBDIR trick
FetchContent_Declare(doctest
  GIT_REPOSITORY https://github.com/doctest/doctest
  GIT_TAG v2.4.12
  GIT_SHALLOW ON
  SOURCE_SUBDIR headers-only)
FetchContent_MakeAvailable(doctest)

# registers every TEST_CASE as its own CTest test
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

function(lab_add_tests TEST_TARGET)
  add_executable(${TEST_TARGET} test_main.cpp common.h ${ARGN})
  target_include_directories(${TEST_TARGET} SYSTEM PRIVATE ${doctest_SOURCE_DIR})
  target_link_libraries(${TEST_TARGET} PRIVATE lab::lab)
  lab_target_defaults(${TEST_TARGET})
  if(NOT MSVC)
    # doctest includes <ciso646>, which libstdc++ answers with a #warning since C++20
    target_compile_options(${TEST_TARGET} PRIVATE -Wno-cpp)
  endif()
endfunction()

# tests that need a GPU (or a software adapter), but no window: they render into textures
lab_add_tests(lab-tests
  test_buffer.cpp
  test_pipeline.cpp
  test_render.cpp
  test_vertex_format.cpp)
doctest_discover_tests(lab-tests)

# Tests that open windows and so need a display. They carry the label "display",
# as do the sample smoke tests: `ctest -LE display` runs everything else.
lab_add_tests(lab-window-tests
  test_window.cpp)
doctest_discover_tests(lab-window-tests PROPERTIES LABELS display RUN_SERIAL ON)
