#ifndef WGPU_LAB_SHADER_H
#define WGPU_LAB_SHADER_H

#include <webgpu/webgpu.hpp>

#include <iostream>

namespace lab {

struct Shader {
  std::string label, source;

  Shader(const std::string& label, const std::string& path);

  // Warning! User is responsible to release the returned shader module
  // - automatically managed by `Pipeline` constructor
  [[nodiscard]] wgpu::ShaderModule transfer(wgpu::Device device);
};

} // namespace lab

#endif // WGPU_LAB_SHADER_H
