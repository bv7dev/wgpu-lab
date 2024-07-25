#include "lab_shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace lab {

Shader::Shader(const char* path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Shader: Could not open " << path << std::endl;
    return;
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  source = buffer.str();
}

void Shader::transfer(wgpu::Device device) {
  wgpu::ShaderModuleWGSLDescriptor wgslDesc = {{
      .chain = {.next = nullptr, .sType = wgpu::SType::ShaderModuleWGSLDescriptor},
      .code = source.c_str(),
  }};

  wgpu::ShaderModuleDescriptor shaderDesc;
  shaderDesc.nextInChain = &wgslDesc.chain;
  shaderDesc.label = "My shader module";

  module = device.createShaderModule(shaderDesc);
}

Shader::~Shader() {
  module.release();
  module = nullptr;
}

} // namespace lab
