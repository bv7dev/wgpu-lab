#include <lab>

struct MyVertexFormat {
  float pos[2];
  float color[3];
};

int main() {
  lab::Webgpu webgpu("My WebGPU Context");
  lab::Shader shader("My Shader", "shaders/draw_colored.wgsl");
  lab::Pipeline pipeline(shader, webgpu);

  std::vector<MyVertexFormat> vertex_data = {
      //         x      y                r     g     b
      {.pos = {-0.5f, -0.5f}, .color = {0.8f, 0.2f, 0.2f}},
      {.pos = {+0.5f, -0.5f}, .color = {0.8f, 0.8f, 0.2f}},
      {.pos = {+0.0f, +0.5f}, .color = {0.2f, 0.8f, 0.4f}},
  };
  lab::Buffer vertex_buffer("My Vertex Buffer", vertex_data, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 0); // position
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x3, 1); // color
  pipeline.finalize();

  lab::Window window("Hello Triangle", 640, 400);
  lab::Surface surface(window, webgpu);

  while (lab::tick()) {
    pipeline.render_frame(surface, 3, 1);
    lab::sleep(50ms);
  }
}
