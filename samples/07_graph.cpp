// 07: several pipelines in one render pass
//     nodes and edges of a graph share one mesh and one uniform buffer, but have their own shaders

#include <lab>

#include <glm/glm.hpp>

#include <cmath>
#include <numbers>
#include <vector>

using enum wgpu::VertexFormat;

struct NodeInstance {
  glm::vec2 pos;
  float scale;
};

struct EdgeInstance {
  glm::vec2 pos_a, pos_b;
  float scale;
};

struct alignas(8) UniformParams {
  glm::vec2 ratio;
  float time;
};

int main() {
  lab::Gpu gpu;
  lab::Window window("graph visualizer", 900, 600);
  lab::Surface surface(gpu, window);

  // a hexagon, drawn with an index buffer: 6 vertices form 4 triangles
  const float k = 2.0f * std::sqrt(3.0f) / 3.0f;
  std::vector<glm::vec2> mesh{
      {-k * 0.5f, 1.0}, {-k, 0.0f}, {-k * 0.5f, -1.0}, // left
      {k * 0.5f, -1.0}, {k, 0.0f},  {k * 0.5f, 1.0},   // right
  };
  std::vector<uint16_t> mesh_indices{
      0, 1, 2, // left   tri
      0, 2, 3, // center tri 1
      3, 5, 0, // center tri 2
      3, 4, 5, // right  tri
  };
  lab::Buffer mesh_vertices(gpu, mesh);
  lab::Buffer mesh_index_buffer(gpu, mesh_indices, wgpu::BufferUsage::Index);

  // the graph: nodes on a circle, every node connected to the next and to the one three further
  const int node_count = 12;
  std::vector<NodeInstance> nodes;
  std::vector<EdgeInstance> edges;
  auto node_position = [&](int i) {
    const float angle = float(i % node_count) / float(node_count) * 2.0f * std::numbers::pi_v<float>;
    return 0.7f * glm::vec2{std::cos(angle), std::sin(angle)};
  };
  for (int i = 0; i < node_count; ++i) {
    nodes.push_back({.pos = node_position(i), .scale = 0.07f});
    edges.push_back({.pos_a = node_position(i), .pos_b = node_position(i + 1), .scale = 0.012f});
    edges.push_back({.pos_a = node_position(i), .pos_b = node_position(i + 3), .scale = 0.006f});
  }
  lab::Buffer node_instances(gpu, nodes);
  lab::Buffer edge_instances(gpu, edges);

  UniformParams uniforms{.ratio = {1.0f / surface.aspect(), 1.0f}, .time = 0.0f};
  lab::Buffer<UniformParams> uniform_buffer(gpu, {uniforms}, wgpu::BufferUsage::Uniform);

  // both pipelines read the mesh the same way and differ in their per-instance data
  const lab::VertexLayout mesh_layout = lab::vertex<glm::vec2>({Float32x2});

  lab::Shader node_shader(gpu, "shaders/07_graph_nodes.wgsl");
  lab::Pipeline node_pipeline(
      gpu, node_shader,
      {.vertex_buffers = {mesh_layout, lab::instance<NodeInstance>({Float32x2, Float32})}, .target = surface});

  lab::Shader edge_shader(gpu, "shaders/07_graph_edges.wgsl");
  lab::Pipeline edge_pipeline(
      gpu, edge_shader,
      {.vertex_buffers = {mesh_layout, lab::instance<EdgeInstance>({Float32x2, Float32x2, Float32})},
       .target = surface});

  // what each pipeline draws: the same mesh and indices, its own instances
  lab::Draw draw_nodes{
      .vertex_buffers = {mesh_vertices, node_instances},
      .index_buffer = mesh_index_buffer,
      .bind_groups = {node_pipeline.bind_group(0, {{0, uniform_buffer}})},
  };
  lab::Draw draw_edges{
      .vertex_buffers = {mesh_vertices, edge_instances},
      .index_buffer = mesh_index_buffer,
      .bind_groups = {edge_pipeline.bind_group(0, {{0, uniform_buffer}})},
  };

  while (lab::tick()) {
    if (window.key_pressed(lab::KeyCode::escape)) {
      window.close();
    }

    uniforms.ratio.x = 1.0f / surface.aspect();
    uniform_buffer.write(uniforms);

    // A render pass collects draw calls into one target. This one is a whole frame:
    // when it goes out of scope, the frame is sent to the gpu and shown in the window.
    lab::RenderPass pass(surface);
    pass.draw(edge_pipeline, draw_edges);
    pass.draw(node_pipeline, draw_nodes); // drawn second, on top of the edges
  }
}
