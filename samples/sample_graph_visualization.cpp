#include <lab>

#include <glm/glm.hpp>

#include <random>

struct Vertex {
  glm::vec2 pos;
};

struct NodeInstance {
  glm::vec2 pos;
  float scale;
};

struct EdgeInstance {
  glm::vec2 pos_a, pos_b;
  float scale;
};

struct alignas(16) UniformParams {
  glm::vec2 ratio;
  float time;
};

int main() {
  lab::Webgpu webgpu("wgpu context");

  lab::Shader node_shader("node shader", "shaders/sample_graph.wgsl");
  lab::Pipeline node_pipeline(node_shader, webgpu);

  lab::Shader edge_shader("edge shader", "shaders/sample_graph_edge_shader.wgsl");
  lab::Pipeline edge_pipeline(edge_shader, webgpu);

  const float h = sqrt(3.0f) / 2.0f;
  std::vector<Vertex> mesh{
      {{-0.5f, h}}, {{-1.0f, 0.0f}}, {{-0.5f, -h}}, // left
      {{0.5f, -h}}, {{1.0f, 0.0f}},  {{0.5f, h}},   // right
  };
  std::vector<uint16_t> mesh_indices{
      0, 1, 2, // left   tri
      0, 2, 3, // center tri 1
      3, 5, 0, // center tri 2
      3, 4, 5, // right  tri
  };

  lab::Buffer<Vertex> mesh_vertex_buffer("mesh vertex buffer", mesh, webgpu);
  lab::Buffer<uint16_t> mesh_index_buffer("mesh index buffer", mesh_indices, wgpu::BufferUsage::Index, webgpu);

  node_pipeline.add_vertex_buffer(mesh_vertex_buffer);
  node_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);

  node_pipeline.add_index_buffer(mesh_index_buffer, wgpu::IndexFormat::Uint16);

  edge_pipeline.add_vertex_buffer(mesh_vertex_buffer);
  edge_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);

  edge_pipeline.add_index_buffer(mesh_index_buffer, wgpu::IndexFormat::Uint16);
  // TODO: add mechanism to avoid duplicate setups like above:
  // idea: add_vertex_buffer({node_pipe, edge_pipe}, mesh_vertex_buffer);

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

  std::vector<EdgeInstance> edge_instances;
  edge_instances.push_back({.pos_a = {0.2f, 0.3f}, .pos_b = {0.8f, 0.5f}, .scale = 0.2f});

  lab::Buffer<EdgeInstance> edge_instance_buffer("edge instance buffer", edge_instances, webgpu);

  edge_pipeline.add_vertex_buffer(edge_instance_buffer, wgpu::VertexStepMode::Instance);
  edge_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 1);
  edge_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 2);
  edge_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32, 3);

  lab::Window window("graph visualizer", 900, 600);

  UniformParams uniform_buffer{.ratio{window.ratio(), 1.0}};
  lab::Buffer<UniformParams> node_uniform_buffer("node uniform buffer", {uniform_buffer},
                                                 wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst, webgpu);

  node_pipeline.add_uniform_buffer(node_uniform_buffer, 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
  node_pipeline.finalize();

  edge_pipeline.add_uniform_buffer(node_uniform_buffer, 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
  edge_pipeline.finalize();

  window.set_key_callback([&window](auto event) {
    if (event.key == lab::KeyCode::escape) window.close();
  });

  lab::Surface surface(window, webgpu);

  window.set_resize_callback([&uniform_buffer, &node_uniform_buffer, &window, &surface](int w, int h) {
    std::cout << "window resized to: " << w << "x" << h << "\n";
    uniform_buffer.ratio.x = static_cast<float>(h) / w;
    node_uniform_buffer.write(uniform_buffer);
    surface.reconfigure(w, h);
  });

  while (lab::tick()) {
    edge_pipeline.render_frame(surface, mesh_indices.size(), edge_instances.size());
    node_pipeline.render_frame(surface, mesh_indices.size(), node_instances.size());
  }
}
