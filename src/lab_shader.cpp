#include "lab_shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace lab {

Shader::Shader(const std::string& lbl, const std::string& path) : label{lbl} {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Shader: Could not open \"" << path << "\"" << std::endl;
    return;
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  source = buffer.str();
}

wgpu::ShaderModule Shader::transfer(wgpu::Device device) {
  wgpu::ShaderModuleWGSLDescriptor wgslDesc = {{
      .chain = {.next = nullptr, .sType = wgpu::SType::ShaderModuleWGSLDescriptor},
      .code = source.c_str(),
  }};
  wgpu::ShaderModuleDescriptor shaderDesc;
  shaderDesc.nextInChain = &wgslDesc.chain;
  shaderDesc.label = "My shader module";
  return device.createShaderModule(shaderDesc);
}

} // namespace lab
