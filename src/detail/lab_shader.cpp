#include <objects/lab_shader.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace lab {

Shader::Shader(const std::string& lbl) : label{lbl} {}

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

wgpu::ShaderModule Shader::transfer(wgpu::Device device, wgpu::SType struct_type) const {
  switch (struct_type) {
  case wgpu::SType::ShaderModuleSPIRVDescriptor:
    std::cout << "Error: Shader: SPIR-V Shader Module not yet implemented. Please use WGSL instead." << std::endl;
    return nullptr;
  case wgpu::SType::ShaderModuleWGSLDescriptor:
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.sType = struct_type;
    wgslDesc.code = source.c_str();
    wgpu::ShaderModuleDescriptor shaderDesc;
    shaderDesc.nextInChain = &wgslDesc;
    shaderDesc.label = label.c_str();
    return device.CreateShaderModule(&shaderDesc);
  }
  std::cout << "Error: Shader: struct_type not supported." << std::endl;
  return nullptr;
}

} // namespace lab
