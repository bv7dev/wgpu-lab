#include <lab>

struct MyVertexFormat {
  float pos[2];
  float color[3];
};

int main() {
  lab::Webgpu webgpu("My WebGPU Context");
  lab::Shader shader("My Shader", "shaders/draw_colored.wgsl");

  lab::Pipeline pipeline(shader, webgpu); // the rendering pipeline

  // colored triangle data
  std::vector<MyVertexFormat> vertex_data = {
      //         X      Y                R     G     B
      {.pos = {-0.5f, -0.5f}, .color = {0.8f, 0.2f, 0.2f}},
      {.pos = {+0.5f, -0.5f}, .color = {0.8f, 0.8f, 0.2f}},
      {.pos = {+0.0f, +0.5f}, .color = {0.2f, 0.8f, 0.4f}},
  };

  // vertex buffer (sends copy of data to GPU memory)
  lab::Buffer vertex_buffer("My Vertex Buffer", vertex_data, webgpu);

  // pipeline needs to know about buffers and their memory layouts (vertex attributes)
  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 0); // position
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x3, 1); // color
  pipeline.finalize();                                          // make ready for rendering

  lab::Window window("Hello Triangle", 640, 400);

  lab::Surface surface(window, webgpu); // surface to render onto

  // main application loop
  while (lab::tick()) {
    pipeline.render_frame(surface, 3, 1); // 3 vertices, 1 instance
  }
}
