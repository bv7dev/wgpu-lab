#ifndef WGPU_LAB_WEBGPU_H
#define WGPU_LAB_WEBGPU_H

#include <string>

#include <dawn/webgpu_cpp.h>

namespace lab {

struct Webgpu {
  Webgpu(const std::string& label, wgpu::PowerPreference = wgpu::PowerPreference::HighPerformance);

  Webgpu(const Webgpu&) = delete;
  Webgpu& operator=(const Webgpu&) = delete;

  ~Webgpu();

  wgpu::SurfaceCapabilities capabilities{};

  wgpu::Instance instance = nullptr;
  wgpu::Adapter adapter = nullptr;
  wgpu::Device device = nullptr;
  wgpu::Queue queue = nullptr;

  std::string label;
};

} // namespace lab

#endif // WGPU_LAB_WEBGPU_H
