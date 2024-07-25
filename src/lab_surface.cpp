#include "lab_surface.h"

#include <glfw3webgpu.h>

namespace lab {

Surface::Surface(Window& wnd, Webgpu& wgpu)
    : window{wnd}, webgpu{wgpu}, wgpu_surface{glfwGetWGPUSurface(wgpu.instance, reinterpret_cast<GLFWwindow*>(wnd.get_handle()))} {
  configure();
}

void Surface::configure() {
  wgpu::SurfaceConfiguration surfaceConfig = {{
      .device = webgpu.device,
      .format = webgpu.capabilities.formats[0],
      .usage = wgpu::TextureUsage::RenderAttachment,
      .alphaMode = wgpu::CompositeAlphaMode::Auto,
      .width = static_cast<uint32_t>(window.width()),
      .height = static_cast<uint32_t>(window.height()),
      .presentMode = wgpu::PresentMode::Fifo,
  }};
  wgpu_surface.configure(surfaceConfig);
}

Surface::~Surface() {
  wgpu_surface.unconfigure();
  wgpu_surface.release();
}

} // namespace lab
