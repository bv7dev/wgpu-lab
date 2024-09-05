#ifndef WGPU_LAB_SHADER_H
#define WGPU_LAB_SHADER_H

#include <dawn/webgpu_cpp.h>

#include <string>

namespace lab {

struct Shader {
  std::string label, source;

  Shader(const std::string& label);

  Shader(const std::string& label, const std::string& path);

  // Warning! User is responsible to `.release()` the returned shader module
  // - be careful not to leak resources if you need to use it directly
  // - by default, it's automatically managed, see: `Pipeline::init()`
  [[nodiscard]] wgpu::ShaderModule transfer(wgpu::Device device) const;
};

} // namespace lab

#endif // WGPU_LAB_SHADER_H
