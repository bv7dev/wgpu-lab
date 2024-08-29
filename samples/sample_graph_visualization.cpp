#include <lab>

#include <glm/glm.hpp>

#include <random>

struct NodeInstance {
  glm::vec2 pos;
  float scale;
};

struct NodeMeshVertex {
  glm::vec2 pos;
};

struct alignas(16) NodeUniformParams {
  glm::vec2 ratio;
  float time;
};

int main() {
  lab::Webgpu webgpu("wgpu context");

  lab::Shader node_shader("node shader", "shaders/sample_graph.wgsl");
  lab::Pipeline node_pipeline(node_shader, webgpu);

  // todo: add index buffer
  float h = sqrt(3.0f) / 2.0f;
  std::vector<NodeMeshVertex> node_mesh{
      {{0.0f, 0.0f}}, {{1.0f, 0.0f}},  {{0.5f, h}},   // top-right
      {{0.0f, 0.0f}}, {{0.5f, h}},     {{-0.5f, h}},  // top-center
      {{0.0f, 0.0f}}, {{-1.0f, 0.0f}}, {{-0.5f, h}},  // top-left
      {{0.0f, 0.0f}}, {{1.0f, 0.0f}},  {{0.5f, -h}},  // bottom-right
      {{0.0f, 0.0f}}, {{0.5f, -h}},    {{-0.5f, -h}}, // bottom-center
      {{0.0f, 0.0f}}, {{-1.0f, 0.0f}}, {{-0.5f, -h}}, // bottom-left
  };
  lab::Buffer<NodeMeshVertex> node_vertex_buffer("node vertex buffer", node_mesh, webgpu);

  node_pipeline.add_vertex_buffer(node_vertex_buffer);
  node_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);

  std::vector<NodeInstance> node_instances;
  node_instances.reserve(1000);

  std::default_random_engine prng{0};
  for (int i = 0; i < node_instances.capacity(); ++i) {
    std::normal_distribution<float> dst_pos{0.0f, 0.2f};
    std::normal_distribution<float> dst_scl{0.02f, 0.005f};
    node_instances.push_back({.pos = {dst_pos(prng), dst_pos(prng)}, .scale = dst_scl(prng)});
  }
  lab::Buffer<NodeInstance> node_instance_buffer("node instance buffer", node_instances, webgpu);

  node_pipeline.add_vertex_buffer(node_instance_buffer, wgpu::VertexStepMode::Instance);
  node_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 1);
  node_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32, 2);

  lab::Window window("graph visualizer", 900, 600);

  NodeUniformParams node_uniform_params{.ratio{window.ratio(), 1.0}};
  lab::Buffer<NodeUniformParams> node_uniform_buffer("node uniform buffer", {node_uniform_params},
                                                     wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst, webgpu);

  node_pipeline.add_uniform_buffer(node_uniform_buffer, 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
  node_pipeline.finalize();

  window.set_key_callback([&window](auto event) {
    if (event.key == lab::KeyCode::escape) window.close();
  });

  lab::Surface surface(window, webgpu);

  window.set_resize_callback([&node_uniform_params, &node_uniform_buffer, &window, &surface](int w, int h) {
    std::cout << "window resized to: " << w << "x" << h << "\n";
    node_uniform_params.ratio.x = static_cast<float>(h) / w;
    node_uniform_buffer.write(node_uniform_params);
    surface.reconfigure(w, h);
  });

  while (lab::tick()) {
    node_pipeline.render_frame(surface, node_mesh.size(), node_instances.size());
  }
}
