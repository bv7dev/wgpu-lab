#include <objects/lab_shader.h>

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace lab {

Shader::Shader(const std::string& lbl) : label{lbl} {}

Shader::Shader(const std::string& lbl, const std::string& path) : label{lbl} {
  std::ifstream file(path);
  if (!file.is_open()) {
    // relative paths depend on where the program is started from, so say where that is
    throw std::runtime_error(std::format("lab::Shader \"{}\": could not open \"{}\" (working directory: {})", lbl, path,
                                         std::filesystem::current_path().string()));
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  source = buffer.str();
}

wgpu::ShaderModule Shader::transfer(wgpu::Device device, wgpu::SType struct_type) const {
  switch (struct_type) {
  case wgpu::SType::ShaderSourceSPIRV:
    std::cout << "Error: Shader: SPIR-V Shader Module not yet implemented. Please use WGSL instead." << std::endl;
    return nullptr;
  case wgpu::SType::ShaderSourceWGSL: {
    wgpu::ShaderSourceWGSL wgslDesc;
    wgslDesc.code = std::string_view(source);
    wgpu::ShaderModuleDescriptor shaderDesc;
    shaderDesc.nextInChain = &wgslDesc;
    shaderDesc.label = std::string_view(label);
    return device.CreateShaderModule(&shaderDesc);
  }
  default:
    break;
  }
  std::cout << "Error: Shader: struct_type not supported." << std::endl;
  return nullptr;
}

} // namespace lab
