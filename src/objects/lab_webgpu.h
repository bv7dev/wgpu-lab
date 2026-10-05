#ifndef WGPU_LAB_WEBGPU_H
#define WGPU_LAB_WEBGPU_H

#include <string>

#include <webgpu/webgpu_cpp.h>

namespace lab {

struct Webgpu {
  Webgpu(const std::string& label, wgpu::PowerPreference = wgpu::PowerPreference::HighPerformance);

  Webgpu(const Webgpu&) = delete;
  Webgpu& operator=(const Webgpu&) = delete;

  ~Webgpu();

  // Texture format shared by all surfaces and pipelines of this instance
  // - BGRA8Unorm is what desktop window systems present natively
  wgpu::TextureFormat surface_format = wgpu::TextureFormat::BGRA8Unorm;

  wgpu::Instance instance = nullptr;
  wgpu::Adapter adapter = nullptr;
  wgpu::Device device = nullptr;
  wgpu::Queue queue = nullptr;

  std::string label;
};

} // namespace lab

#endif // WGPU_LAB_WEBGPU_H
