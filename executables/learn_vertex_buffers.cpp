#include <lab>

// todo: it seems like my webgpu error callback is not working, so i don't get any error messages

int main() {
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My Shader", "shaders/vbuf.wgsl");

  lab::Window window("A Window with tutorial code", 640, 400);

  std::vector<float> vertex_data = {-0.5,   -0.5, +0.5,   -0.5, +0.0,   +0.5,
                                    -0.55f, -0.5, -0.05f, +0.5, -0.55f, +0.5};
  uint32_t vertex_count = static_cast<uint32_t>(vertex_data.size() / 2);

  lab::ReadableBuffer<float> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  lab::Surface surface(window, webgpu);
  lab::Pipeline pipeline(shader, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer.wgpu_buffer, 0);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);

  pipeline.finalize();

  window.set_resize_callback(
      [&surface](int width, int height) { surface.reconfigure(width, height); });

  while (lab::tick()) {
    pipeline.render_frame(surface, {vertex_count, 1});
    lab::sleep(50ms);
  }
}
