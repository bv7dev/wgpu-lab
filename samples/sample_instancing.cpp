#include <lab>

#include <cmath>

struct Position {
  float x, y;
};

std::vector<Position> make_more(const std::vector<Position>& A, const std::vector<Position>& B) {
  std::vector<Position> result;
  for (auto& b : B) {
    for (auto& a : A) {
      result.push_back({a.x + b.x, a.y + b.y});
    }
  }
  return result;
}

int main() {
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My Shader", "shaders/test_instancing.wgsl");
  lab::Pipeline pipeline(shader, webgpu);

  lab::Window window("Instancing Experiment", 512, 512);
  lab::Surface surface(window, webgpu);

  std::vector<Position> vertex_data{{0.f, 1.f}, {-sqrtf(3.f) / 2.f, -3.f / 6.f}, {sqrtf(3.f) / 2.f, -3.f / 6.f}};

  auto more = make_more(vertex_data, vertex_data);
  auto even_more = make_more(more, vertex_data);
  auto instance_data = make_more(even_more, vertex_data);

  lab::Buffer<Position> vertex_buffer("My vertex buffer", vertex_data, webgpu);
  lab::Buffer<Position> instance_positions("My instance positions", instance_data, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer.wgpu_buffer);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 0);

  pipeline.add_vertex_buffer(instance_positions.wgpu_buffer, wgpu::VertexStepMode::Instance);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 1);

  pipeline.finalize();

  while (lab::tick()) {
    pipeline.render_frame(surface,
                          {static_cast<uint32_t>(vertex_data.size()), static_cast<uint32_t>(instance_data.size())});
    lab::sleep(50ms);
  }
}
