#include "lab_detail.h"

#include "platform/lab_platform.h"

#include <lab_shader.h>

#include <fstream>
#include <sstream>

namespace lab {

namespace {

// A relative path is tried from the working directory first and then from the
// directory of the executable, so a program finds its shaders wherever it is started from.
std::filesystem::path locate(const std::filesystem::path& file, const std::string& label) {
  std::error_code ignored;
  if (std::filesystem::is_regular_file(file, ignored)) {
    return file;
  }
  std::string tried = std::format("\"{}\"", std::filesystem::absolute(file, ignored).string());
  if (file.is_relative()) {
    const std::filesystem::path beside_executable = platform::executable_directory() / file;
    if (std::filesystem::is_regular_file(beside_executable, ignored)) {
      return beside_executable;
    }
    tried += std::format(" and \"{}\"", beside_executable.string());
  }
  detail::fail(label, std::format("file not found, looked for {}", tried));
}

} // namespace

Shader::Shader(Gpu& gpu, const std::filesystem::path& wgsl_file, std::string_view label)
    : name{label.empty() ? wgsl_file.filename().string() : std::string{label}} {
  const std::filesystem::path path = locate(wgsl_file, name);
  std::ifstream file(path);
  if (!file) {
    detail::fail(name, std::format("could not read \"{}\"", path.string()));
  }
  std::stringstream source;
  source << file.rdbuf();
  compile(gpu, source.str());
}

Shader Shader::from_source(Gpu& gpu, std::string_view wgsl, std::string_view label) {
  Shader shader;
  shader.name = label;
  shader.compile(gpu, wgsl);
  return shader;
}

void Shader::compile(Gpu& gpu, std::string_view wgsl) {
  wgpu::ShaderSourceWGSL source;
  source.code = wgsl;
  wgpu::ShaderModuleDescriptor desc;
  desc.nextInChain = &source;
  desc.label = std::string_view(name);

  // the error WebGPU reports for a shader that does not compile contains the compiler's
  // diagnostics with line and column, it only lacks the name of the shader
  if (auto error = detail::capture_error(*gpu.state(), [&] { module = gpu.device().CreateShaderModule(&desc); })) {
    detail::fail(name, std::format("does not compile\n{}", *error));
  }
}

} // namespace lab
