#include <lab>

int main() {
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My Shader", "shaders/vbuf2.wgsl");

  lab::Window window("Vertex Buffer Demo", 640, 400);
  lab::Surface surface(window, webgpu);

  std::vector<float> vertex_data = {-0.5f,  -0.5f, 1.0f, +0.5f,  -0.5f, 0.6f, +0.0f,  0.5f, 0.2f,
                                    -0.55f, -0.5f, 1.0f, -0.05f, +0.5f, 0.6f, -0.55f, 0.5f, 0.2f};
  lab::Buffer<float> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  lab::Pipeline pipeline(shader, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer.wgpu_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32, 1);

  pipeline.finalize();

  while (lab::tick()) {
    pipeline.render_frame(surface, {3, 1});
    lab::sleep(50ms);
  }
}
