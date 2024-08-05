#include <lab>

#include <cmath>

int main() {
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My Shader", "shaders/test_instancing.wgsl");
  lab::Pipeline pipeline(shader, webgpu);

  lab::Window window("Instancing Experiment", 512, 512);
  lab::Surface surface(window, webgpu);

  struct Position {
    float x, y;
  };
  using Vertex = Position;

  std::vector<Vertex> vertex_data{
      {0.f, 1.f}, {-sqrtf(3.f) / 2.f, -3.f / 6.f}, {sqrtf(3.f) / 2.f, -3.f / 6.f}};

  lab::Buffer<Vertex> vertex_buffer("My vertex buffer", vertex_data, webgpu);
  lab::Buffer<Position> instance_positions("My instance positions", vertex_data, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer.wgpu_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);

  pipeline.add_vertex_buffer(instance_positions.wgpu_buffer, wgpu::VertexStepMode::Instance);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 1);

  pipeline.finalize();

  window.set_resize_callback(
      [&surface](int width, int height) { surface.reconfigure(width, height); });

  while (lab::tick()) {
    pipeline.render_frame(surface, {3, 3});
    lab::sleep(50ms);
  }
}
