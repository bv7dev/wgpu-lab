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
  void configure_surface(uint32_t width, uint32_t height);
  void create_pipeline();

  bool render_frame();

  ~Webgpu();

  wgpu::Instance instance;
  wgpu::Device device;
  wgpu::Surface surface;
  wgpu::Queue queue;
  wgpu::RenderPipeline pipeline;
  wgpu::SurfaceCapabilities capabilities;
};

} // namespace lab

#endif // WGPU_LAB_WEBGPU_H
