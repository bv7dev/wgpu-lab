#ifndef WGPU_LAB_WEBGPU_H
#define WGPU_LAB_WEBGPU_H

#include <webgpu/webgpu.hpp>

namespace lab {

struct Webgpu {
  Webgpu(const char* label);

  Webgpu(const Webgpu&) = delete;
  Webgpu& operator=(const Webgpu&) = delete;

  void create_pipeline(wgpu::ShaderModule);

  void render_frame(wgpu::Surface);

  ~Webgpu();

  wgpu::SurfaceCapabilities capabilities;

  wgpu::Instance instance;
  wgpu::Device device;
  wgpu::RenderPipeline pipeline;
  wgpu::Queue queue;

  const char* label;
};

} // namespace lab

#endif // WGPU_LAB_WEBGPU_H
