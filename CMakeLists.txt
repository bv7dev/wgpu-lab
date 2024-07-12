cmake_minimum_required(VERSION 3.4...3.28 FATAL_ERROR)
project(lab VERSION 0.1.0)

add_subdirectory(libs/glfw-3.4)
add_subdirectory(libs/webgpu-dawn)

add_executable(lab src/main.cpp)
target_link_libraries(lab glfw webgpu)

set_target_properties(lab PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
	target_compile_options(lab PRIVATE /W4)
else()
	target_compile_options(lab PRIVATE -Wall -Wextra -pedantic)
endif()
