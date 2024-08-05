#include <lab>

// todo: it seems like my webgpu error callback is not working, so i don't get any error messages

int main() {
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My Shader", "shaders/vbuf2.wgsl");
  lab::Pipeline pipeline(shader, webgpu);

  lab::Window window("Vertex Buffer Demo", 640, 400);
  lab::Surface surface(window, webgpu);

  std::vector<float> vertex_data = {-0.5f,  -0.5f, 1.0f, +0.5f,  -0.5f, 0.6f, +0.0f,  0.5f, 0.2f,
                                    -0.55f, -0.5f, 1.0f, -0.05f, +0.5f, 0.6f, -0.55f, 0.5f, 0.2f};
  uint32_t vertex_count = static_cast<uint32_t>(vertex_data.size() / 3);

  lab::Buffer<float> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer.wgpu_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);

  // todo: auto compute stride from all previous attributes
  // ... could be non-computable if using multiple buffers,
  // except we have buffer <-> attrib association
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32, 1, 2 * sizeof(float));

  pipeline.finalize();

  window.set_resize_callback(
      [&surface](int width, int height) { surface.reconfigure(width, height); });

  while (lab::tick()) {
    pipeline.render_frame(surface, {vertex_count, 1});
    lab::sleep(50ms);
  }
}