#ifndef WGPU_LAB_SHADER_H
#define WGPU_LAB_SHADER_H

#include <webgpu/webgpu.hpp>

#include <iostream>

namespace lab {

struct Shader {
  wgpu::ShaderModule module;
  std::string source;

  Shader(const char* path);

  void transfer(wgpu::Device device);

  ~Shader();
};

} // namespace lab

#endif // WGPU_LAB_SHADER_H
