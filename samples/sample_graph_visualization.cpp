#include <lab>

#include <glm/glm.hpp>

struct NodeInstance {
  glm::vec2 pos;
  float scale;
  uint8_t r, g, b, a;
};

struct NodeMeshVertex {
  glm::vec2 pos;
};

struct alignas(16) NodeUniformParams {
  glm::vec2 ratio;
  float time;
};

int main() {
  lab::Window window("graph visualizer", 900, 600);
  window.set_key_callback([&window](auto event) {
    if (event.key == lab::KeyCode::escape) window.close();
  });

  lab::Webgpu webgpu("wgpu context");
  lab::Surface surface(window, webgpu);

  lab::Shader node_shader("node shader", "shaders/sample_graph.wgsl");
  lab::Pipeline node_pipeline(node_shader, webgpu);

  NodeUniformParams node_uniform_params{.ratio{window.ratio(), 1.0}};
  lab::Buffer<NodeUniformParams> node_uniform_buffer("node uniform buffer", {node_uniform_params},
                                                     wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst, webgpu);

  node_pipeline.add_uniform_buffer(node_uniform_buffer, 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);

  window.set_resize_callback([&node_uniform_params, &node_uniform_buffer, &window](int w, int h) {
    std::cout << "window resized to: " << w << "x" << h << "\n";
    node_uniform_params.ratio.x = static_cast<float>(h) / w;
    node_uniform_buffer.write(node_uniform_params);
  });

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
  lab::Buffer<NodeMeshVertex> node_vertex_buffer("node vertex buffer", webgpu);
  node_vertex_buffer.to_device(
      [&node_mesh](auto vmap) {
        for (int i = 0; i < node_mesh.size(); ++i) {
          vmap.push(node_mesh[i]);
        }
      },
      node_mesh.size(), wgpu::BufferUsage::Vertex);

  node_pipeline.add_vertex_buffer(node_vertex_buffer);
  node_pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);

  lab::Buffer<glm::vec2> node_instance_buffer("node instance buffer", webgpu);

  node_pipeline.finalize();

  while (lab::tick()) {
    node_pipeline.render_frame(surface, node_mesh.size(), 1);
  }
}
