#include "lab_surface.h"

#include <glfw3webgpu.h>

namespace lab {

Surface::Surface(Window& wnd, Webgpu& wgpu)
    : window{wnd}, webgpu{wgpu}, wgpu_surface{glfwGetWGPUSurface(
                                     wgpu.wgpu_instance,
                                     reinterpret_cast<GLFWwindow*>(
                                         wnd.get_handle()))} {
  reconfigure();
}

void Surface::reconfigure(int w, int h) {
  wgpu::SurfaceConfiguration surfaceConfig = {{
      .device = webgpu.device,
      .format = webgpu.capabilities.formats[0],
      .usage = wgpu::TextureUsage::RenderAttachment,
      .alphaMode = wgpu::CompositeAlphaMode::Auto,
      .width = static_cast<uint32_t>(w),
      .height = static_cast<uint32_t>(h),
      .presentMode = wgpu::PresentMode::Fifo,
  }};
  wgpu_surface.configure(surfaceConfig);
}

void Surface::reconfigure() { reconfigure(window.width(), window.height()); }

Surface::~Surface() {
  if (wgpu_surface) {
    wgpu_surface.unconfigure();
    wgpu_surface.release();
    wgpu_surface = nullptr;
  }
}

} // namespace lab
