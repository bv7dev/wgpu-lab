#ifndef WGPU_LAB_WEBGPU_H
#define WGPU_LAB_WEBGPU_H

#include <webgpu/webgpu.hpp>

namespace lab {

struct Webgpu {
  Webgpu(const char* label);

  Webgpu(const Webgpu&) = delete;
  Webgpu& operator=(const Webgpu&) = delete;

  ~Webgpu();

  wgpu::SurfaceCapabilities capabilities;

  wgpu::Instance wgpu_instance = nullptr;
  wgpu::Device device = nullptr;
  wgpu::RenderPipeline pipeline = nullptr;
  wgpu::Queue queue = nullptr;

  const char* label;
};

} // namespace lab

#endif // WGPU_LAB_WEBGPU_H