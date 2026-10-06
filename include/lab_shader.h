#ifndef WGPU_LAB_SHADER_H
#define WGPU_LAB_SHADER_H

#include <lab_gpu.h>

#include <filesystem>
#include <string>
#include <string_view>

namespace lab {

// A compiled WGSL shader module.
// ```cpp
// lab::Shader shader(gpu, "shaders/triangle.wgsl");
// auto other = lab::Shader::from_source(gpu, "@vertex fn vs_main() ...");
// ```
class Shader {
public:
  // Loads and compiles a WGSL file
  //  - a relative path is looked up from the working directory first,
  //    then from the directory of the executable
  //  - throws lab::Error if the file is not found or does not compile,
  //    the message contains the compiler's diagnostics
  Shader(Gpu& gpu, const std::filesystem::path& wgsl_file, std::string_view label = {});

  // Compiles WGSL source code, throws lab::Error if it does not compile
  static Shader from_source(Gpu& gpu, std::string_view wgsl, std::string_view label = "shader");

  Shader(Shader&&) = default;
  Shader& operator=(Shader&&) = default;

  const wgpu::ShaderModule& handle() const { return module; }
  const std::string& label() const { return name; }

private:
  Shader() = default;
  void compile(Gpu& gpu, std::string_view wgsl);

  wgpu::ShaderModule module;
  std::string name;
};

} // namespace lab

#endif // WGPU_LAB_SHADER_H
