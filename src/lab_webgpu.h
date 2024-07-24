#ifndef WGPU_LAB_WEBGPU_H
#define WGPU_LAB_WEBGPU_H

#include <iostream>

#include <webgpu/webgpu.hpp>

namespace lab {

struct Webgpu {
  Webgpu();

  Webgpu(const Webgpu&) = delete;
  Webgpu& operator=(const Webgpu&) = delete;

  void init();
  wgpu::Surface configure_surface(wgpu::Surface surface, uint32_t width, uint32_t height);
  void create_pipeline();

  bool render_frame();

  ~Webgpu();

  wgpu::SurfaceCapabilities capabilities;
  wgpu::Instance instance;
  wgpu::Device device;
  wgpu::Surface surface;
  wgpu::Queue queue;
  wgpu::RenderPipeline pipeline;
};

} // namespace lab

#endif // WGPU_LAB_WEBGPU_H
