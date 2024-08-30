#include <glm/glm.hpp>
#include <lab>
#include <random>

struct EdgeVertex {
  glm::vec2 pos;
};

struct EdgeInstance {
  glm::vec2 pos_a, pos_b;
  float scale;
};

struct alignas(16) EdgeUniformParams {
  glm::vec2 ratio;
  float time;
};

int main() {
  lab::Webgpu webgpu("wgpu context");

  wgpu::SupportedLimits supportedLimits;

  webgpu.adapter.GetLimits(&supportedLimits);
  std::cout << "adapter.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;

  webgpu.device.GetLimits(&supportedLimits);
  std::cout << "device.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;

  lab::Shader edge_shader("edge shader", "shaders/sample_graph_edge_shader.wgsl");
  lab::Pipeline edge_pipeline(edge_shader, webgpu);

  // todo: add index buffer
  const float h = sqrt(3.0f) / 2.0f;
  std::vector<EdgeVertex> node_mesh{
      {{-0.5f, h}}, {{-1.0f, 0.0f}}, {{-0.5f, -h}}, // left
      {{0.5f, -h}}, {{1.0f, 0.0f}},  {{0.5f, h}},   // right
  };
  lab::Buffer<EdgeVertex> edge_vertex_buffer("edge vertex buffer", node_mesh, webgpu);

  std::vector<uint16_t> edge_mesh_indices{0, 1, 2, 0, 2, 3, 3, 5, 0, 3, 4, 5};
  lab::Buffer<uint16_t> edge_mesh_index_buffer("edge mesh index buffer", edge_mesh_indices, wgpu::BufferUsage::Index,
                                               webgpu);

  edge_pipeline.add_vertex_buffer(edge_vertex_buffer);
  edge_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);

  edge_pipeline.add_index_buffer(edge_mesh_index_buffer, wgpu::IndexFormat::Uint16);

  std::vector<EdgeInstance> node_instances;
  node_instances.push_back({.pos_a = {0.2f, 0.3f}, .pos_b = {0.8f, 0.5f}, .scale = 0.05f});

  lab::Buffer<EdgeInstance> edge_instance_buffer("edge instance buffer", node_instances, webgpu);

  edge_pipeline.add_vertex_buffer(edge_instance_buffer, wgpu::VertexStepMode::Instance);
  edge_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 1);
  edge_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 2);
  edge_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32, 3); // todo: scale not working

  lab::Window window("graph visualizer", 900, 600);
  lab::Surface surface(window, webgpu);

  EdgeUniformParams edge_uniform_params{.ratio{window.ratio(), 1.0}};
  lab::Buffer<EdgeUniformParams> edge_uniform_buffer("edge uniform buffer", {edge_uniform_params},
                                                     wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst, webgpu);

  edge_pipeline.add_uniform_buffer(edge_uniform_buffer, 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
  edge_pipeline.finalize();

  window.set_key_callback([&window](auto event) {
    if (event.key == lab::KeyCode::escape) window.close();
  });

  window.set_resize_callback([&edge_uniform_params, &edge_uniform_buffer, &window, &surface](int w, int h) {
    std::cout << "window resized to: " << w << "x" << h << "\n";
    edge_uniform_params.ratio.x = static_cast<float>(h) / w;
    edge_uniform_buffer.write(edge_uniform_params);
    surface.reconfigure(w, h);
  });

  while (lab::tick()) {
    edge_pipeline.render_frame(surface, edge_mesh_indices.size(), node_instances.size());
  }
}
