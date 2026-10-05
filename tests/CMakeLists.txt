# doctest is used header-only and therefore only downloaded,
# see samples/CMakeLists.txt for the SOURCE_SUBDIR trick
FetchContent_Declare(doctest
  GIT_REPOSITORY https://github.com/doctest/doctest
  GIT_TAG v2.4.12
  GIT_SHALLOW ON
  SOURCE_SUBDIR headers-only)
FetchContent_MakeAvailable(doctest)

# tests of the library: they need a GPU (or a software adapter), but no window
add_executable(lab-tests
  test_main.cpp
  test_buffer.cpp
  test_pipeline.cpp
  test_render.cpp
  test_vertex_format.cpp
  offscreen.h)
target_include_directories(lab-tests SYSTEM PRIVATE ${doctest_SOURCE_DIR})
target_link_libraries(lab-tests PRIVATE lab::lab)
lab_target_defaults(lab-tests)

# registers every TEST_CASE as its own CTest test
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(lab-tests)
