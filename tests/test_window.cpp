#include <lab>

namespace lab {

const char* _SHADER_SOURCE = R"(
  @vertex
  fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
      var p = vec2f(0.0, 0.0);
      if (in_vertex_index == 0u) {
          p = vec2f(-0.5, -0.5);
      } else if (in_vertex_index == 1u) {
          p = vec2f(0.5, -0.5);
      } else {
          p = vec2f(0.0, 0.5);
      }
      return vec4f(p, 0.0, 1.0);
  }

  @fragment
  fn fs_main() -> @location(0) vec4f {
      return vec4f(0.0, 0.4, 1.0, 1.0);
  }
  )";

struct Shader {
  wgpu::ShaderModule module;

  Shader(wgpu::Device device) {
    wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc = {{
        .chain = {.next = nullptr, .sType = wgpu::SType::ShaderModuleWGSLDescriptor},
        .code = _SHADER_SOURCE,
    }};

    wgpu::ShaderModuleDescriptor shaderDesc;
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    shaderDesc.label = "My shader module";

    module = device.createShaderModule(shaderDesc);
  }

  ~Shader() { module.release(); }
};

} // namespace lab

int main() {
  lab::Window window{640, 400, "My Window"};

  lab::Webgpu webgpu{"My instance"};

  lab::Shader shader{webgpu.device};

  webgpu.create_pipeline(shader.module);

  // lab::load(webgpu, shader); --> load to signal transfer data to gpu

  lab::Surface surface{window, webgpu};

  // lab::create_surface(window, webgpu);

  while (lab::tick()) {
    webgpu.render_frame(surface.wgpu_surface);

    // lab::render(webgpu, surface);

    lab::sleep(16ms);
  }

  return 0;
}
