#ifndef WGPU_LAB_PLATFORM_H
#define WGPU_LAB_PLATFORM_H

#include <webgpu/webgpu_cpp.h>

#include <filesystem>

struct GLFWwindow;

// Everything that differs between window systems lives behind these functions.
// Supporting a new platform means implementing them for it.

namespace lab::platform {

// Creates the wgpu::Surface which presents into the given GLFW window
// - returns nullptr and prints an error if the window system is not supported
wgpu::Surface create_surface(const wgpu::Instance& instance, GLFWwindow* window);

// Called right after a window was created, for platform specific setup
void window_created(GLFWwindow* window);

// Shows what was rendered into the surface. On the web the browser does that by
// itself when the frame is over, and the surface must not be told to.
void present(const wgpu::Surface& surface);

// Called at the end of every lab::tick(). On the web this is where the program
// hands control back to the browser until the next frame.
void end_of_tick();

// The path of the running executable
// - returns an empty path if it cannot be determined
std::filesystem::path executable_path();

// The directory that contains the running executable
// - returns the working directory if it cannot be determined
std::filesystem::path executable_directory();

} // namespace lab::platform

#endif // WGPU_LAB_PLATFORM_H
