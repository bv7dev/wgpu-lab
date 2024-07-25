#ifndef WGPU_LAB_SHADER_H
#define WGPU_LAB_SHADER_H

#include <webgpu/webgpu.hpp>

#include <iostream>

namespace lab {

struct Shader {
  std::string label;
  std::string source;
  wgpu::ShaderModule module;

  Shader(const std::string& label, const std::string& path);

  wgpu::ShaderModule transfer(wgpu::Device device) const;

  ~Shader();
};

} // namespace lab

#endif // WGPU_LAB_SHADER_H