#include <objects/lab_surface.h>

#include <GLFW/glfw3.h>
#include <webgpu/webgpu_glfw.h>

#include <iostream>

namespace lab {

Surface::Surface(Window& wnd, Webgpu& wgpu)
    : window{wnd}, webgpu{wgpu},
      wgpu_surface{wgpu::glfw::CreateSurfaceForWindow(wgpu.instance, wnd.glfw_window_handle)} {
  reconfigure();
  window.set_resize_callback([&](int width, int height) {
    if (width > 0 && height > 0) {
      this->reconfigure(width, height);
    }
  });
}

void Surface::reconfigure(int w, int h) {
  wgpu::SurfaceConfiguration surfaceConfig = {
      .device = webgpu.device,
      .format = webgpu.capabilities.formats[0],
      .usage = wgpu::TextureUsage::RenderAttachment,
      .alphaMode = wgpu::CompositeAlphaMode::Auto,
      .width = static_cast<uint32_t>(w),
      .height = static_cast<uint32_t>(h),
      .presentMode = wgpu::PresentMode::Fifo,
  };
  wgpu_surface.Configure(&surfaceConfig);
}

void Surface::reconfigure() { reconfigure(window.width(), window.height()); }

wgpu::TextureView Surface::get_current_texture() {
  wgpu::SurfaceTexture surfaceTexture;
  wgpu_surface.GetCurrentTexture(&surfaceTexture);
  if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
    std::cerr << "Error: Pipeline: Could not get current render texture" << std::endl;
    return nullptr;
  }
  wgpu::TextureViewDescriptor viewDescriptor{
      .label = "lab default texture view",
      .format = surfaceTexture.texture.GetFormat(),
      .dimension = wgpu::TextureViewDimension::e2D,
      .baseMipLevel = 0,
      .mipLevelCount = 1,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .aspect = wgpu::TextureAspect::All,
  };
  return surfaceTexture.texture.CreateView(&viewDescriptor);
}

Surface::~Surface() {
  if (wgpu_surface) {
    wgpu_surface.Unconfigure();
    wgpu_surface = nullptr;
  }
}

} // namespace lab
