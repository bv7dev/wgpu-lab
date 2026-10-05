// 06: one mesh drawn many times, with a second vertex buffer that advances per instance
//     arrow keys move the selected triangle, space selects the next one

#include <lab>

#include <glm/glm.hpp>

#include <cmath>
#include <vector>

using enum wgpu::VertexFormat;

struct MyVertex {
  glm::vec2 pos;
};

struct MyInstance {
  glm::vec2 pos;
};

// WGSL aligns a struct to its largest member, vec2f is aligned to 8 bytes:
// the struct in the shader is 24 bytes, alignas makes this one 24 bytes as well
struct alignas(8) MyUniforms {
  glm::vec2 ratio;
  float time;
  float scale;
  uint32_t selected;
};

int main() {
  lab::Gpu gpu;
  lab::Window window("arrow keys move, space selects the next triangle", 640, 400);
  lab::Surface surface(gpu, window);

  // the mesh: an equilateral triangle
  std::vector<MyVertex> vertex_data = {
      {{0.f, 1.f}}, {{-std::sqrt(3.f) / 2.f, -0.5f}}, {{+std::sqrt(3.f) / 2.f, -0.5f}}};
  lab::Buffer vertices(gpu, vertex_data);

  // one position per triangle
  std::vector<MyInstance> instance_data = {{{0.f, 0.f}}, {{.5f, .5f}}, {{-.2f, .4f}}, {{-.7f, -.2f}}, {{.5f, -.6f}}};
  lab::Buffer instances(gpu, instance_data);

  MyUniforms uniforms{.ratio = {1.0f / surface.aspect(), 1.f}, .time = 0.f, .scale = .2f, .selected = 0};
  lab::Buffer<MyUniforms> uniform_buffer(gpu, {uniforms}, wgpu::BufferUsage::Uniform);

  lab::Shader shader(gpu, "shaders/06_instancing.wgsl");
  lab::Pipeline pipeline(
      gpu, shader,
      {.vertex_buffers = {lab::vertex<MyVertex>({Float32x2}),      // @location(0), advances per vertex
                          lab::instance<MyInstance>({Float32x2})}, // @location(1), advances per instance
       .target = surface});

  // the draw call takes its counts from the buffers: 3 vertices, 5 instances
  lab::Draw triangles{
      .vertex_buffers = {vertices, instances},
      .bind_groups = {pipeline.bind_group(0, {{0, uniform_buffer}})},
  };

  const float force = 4.f, friction = 1.f;
  glm::vec2 velocity = {0.f, 0.f};

  while (lab::tick()) {
    if (window.key_pressed(lab::KeyCode::escape)) {
      window.close();
    }
    if (window.key_pressed(lab::KeyCode::space)) {
      uniforms.selected = (uniforms.selected + 1) % instance_data.size();
    }

    // the arrow keys accelerate the selected triangle
    const float dt = lab::delta_seconds();
    const glm::vec2 axis{float(window.key(lab::KeyCode::right)) - float(window.key(lab::KeyCode::left)),
                         float(window.key(lab::KeyCode::up)) - float(window.key(lab::KeyCode::down))};
    velocity = velocity * (1.f - friction * dt) + axis * force * dt;

    // only the element that changed is sent to the gpu
    MyInstance& selected = instance_data[uniforms.selected];
    selected.pos += velocity * dt;
    instances.write(selected, uniforms.selected);

    uniforms.ratio.x = 1.0f / surface.aspect();
    uniforms.time = lab::elapsed_seconds();
    uniform_buffer.write(uniforms);

    pipeline.render_frame(surface, triangles);
  }
}
