#include <platform/lab_platform.h>

#include <GLFW/glfw3.h>

#include <iostream>

#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#elif defined(__linux__)

// GLFW's native access functions are declared by hand instead of including
// <GLFW/glfw3native.h>. That header pulls in Xlib.h, whose macros (Success, None,
// Bool, Status, ...) collide with names used in webgpu_cpp.h, and it requires the
// X11 and Wayland development headers to be installed.
extern "C" {
#if defined(LAB_USE_WAYLAND)
struct wl_display* glfwGetWaylandDisplay(void);
struct wl_surface* glfwGetWaylandWindow(GLFWwindow* window);
#endif
#if defined(LAB_USE_X11)
struct _XDisplay* glfwGetX11Display(void);
unsigned long glfwGetX11Window(GLFWwindow* window);
#endif
}

#else
#error "wgpu-lab: creating a surface is not implemented for this platform yet, see src/platform/lab_surface_glfw.cpp"
#endif

namespace lab::platform {

wgpu::Surface create_surface(const wgpu::Instance& instance, GLFWwindow* window) {
  wgpu::SurfaceDescriptor descriptor{.label = "lab surface"};

#if defined(_WIN32)

  wgpu::SurfaceSourceWindowsHWND source;
  source.hinstance = GetModuleHandleW(nullptr);
  source.hwnd = glfwGetWin32Window(window);
  descriptor.nextInChain = &source;
  return instance.CreateSurface(&descriptor);

#elif defined(__linux__)

  switch (glfwGetPlatform()) {
#if defined(LAB_USE_WAYLAND)
  case GLFW_PLATFORM_WAYLAND: {
    wgpu::SurfaceSourceWaylandSurface source;
    source.display = glfwGetWaylandDisplay();
    source.surface = glfwGetWaylandWindow(window);
    descriptor.nextInChain = &source;
    return instance.CreateSurface(&descriptor);
  }
#endif
#if defined(LAB_USE_X11)
  case GLFW_PLATFORM_X11: {
    wgpu::SurfaceSourceXlibWindow source;
    source.display = glfwGetX11Display();
    source.window = glfwGetX11Window(window);
    descriptor.nextInChain = &source;
    return instance.CreateSurface(&descriptor);
  }
#endif
  default:
    std::cerr << "Error: Surface: wgpu-lab was built without support for this window system "
                 "(see LAB_USE_WAYLAND / LAB_USE_X11)"
              << std::endl;
    return nullptr;
  }

#endif
}

} // namespace lab::platform
