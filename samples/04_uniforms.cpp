// 04: a uniform buffer passes values to the shader that change over time

#include <lab>

#include <cmath>
#include <vector>

struct MyVertex {
  float x, y;
};

// has to match `struct Uniforms` in the shader
struct MyUniforms {
  float ratio[2]; // scales x, so that the triangle keeps its shape whatever the shape of the window
  float time;
  float scale;
};

int main() {
  lab::Gpu gpu;
  lab::Window window("Uniforms", 640, 400);
  lab::Surface surface(gpu, window);

  // an equilateral triangle
  std::vector<MyVertex> vertex_data = {{0.f, 1.f}, {-std::sqrt(3.f) / 2.f, -0.5f}, {std::sqrt(3.f) / 2.f, -0.5f}};
  lab::Buffer vertices(gpu, vertex_data);

  MyUniforms uniforms{.ratio = {1.0f / surface.aspect(), 1.0f}, .time = 0.0f, .scale = 0.4f};
  lab::Buffer<MyUniforms> uniform_buffer(gpu, {uniforms}, wgpu::BufferUsage::Uniform);

  lab::Shader shader(gpu, "shaders/04_uniforms.wgsl");
  lab::Pipeline pipeline(
      gpu, shader, {.vertex_buffers = {lab::vertex<MyVertex>({wgpu::VertexFormat::Float32x2})}, .target = surface});

  // The shader declares `@group(0) @binding(0) var<uniform> uniforms: Uniforms;`
  // A bind group tells the pipeline which buffer that is.
  wgpu::BindGroup bind_group = pipeline.bind_group(0, {{0, uniform_buffer}});

  // everything a draw call needs, set up once and reused every frame
  lab::Draw triangle{.vertex_buffers = {vertices}, .bind_groups = {bind_group}};

  while (lab::tick()) {
    // start the animation over every 7 seconds
    if (lab::elapsed_seconds() > 7.0f) {
      lab::restart_timer();
    }

    // send the current values to the gpu
    uniforms.ratio[0] = 1.0f / surface.aspect(); // follows the size of the window
    uniforms.time = lab::elapsed_seconds();
    uniform_buffer.write(uniforms);

    pipeline.render_frame(surface, triangle);
  }
}
