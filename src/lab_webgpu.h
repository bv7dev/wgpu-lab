#ifndef WGPU_LAB_WEBGPU_H
#define WGPU_LAB_WEBGPU_H

#include <webgpu/webgpu.hpp>

namespace lab {

struct Webgpu {
  Webgpu();

  Webgpu(const Webgpu&) = delete;
  Webgpu& operator=(const Webgpu&) = delete;

  void init(wgpu::Surface surface);
  void create_pipeline();

  void render_frame(wgpu::Surface surface);

  ~Webgpu();

  wgpu::SurfaceCapabilities capabilities;

  wgpu::Instance instance;
  wgpu::Device device;
  wgpu::RenderPipeline pipeline;
  wgpu::Queue queue;
};

} // namespace lab

#endif // WGPU_LAB_WEBGPU_H
