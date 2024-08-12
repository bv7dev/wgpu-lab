#include <lab>

int main() {
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My Shader", "shaders/test_uniform.wgsl");
  lab::Pipeline pipeline(shader, webgpu);

  lab::Window window("Vertex Buffer Demo", 640, 400);
  lab::Surface surface(window, webgpu);

  std::vector<float> vertex_data = {-0.5f,  -0.5f, 1.0f, +0.5f,  -0.5f, 0.6f, +0.0f,  0.5f, 0.2f,
                                    -0.55f, -0.5f, 1.0f, -0.05f, +0.5f, 0.6f, -0.55f, 0.5f, 0.2f};
  uint32_t vertex_count = static_cast<uint32_t>(vertex_data.size() / 3);

  lab::Buffer<float> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  lab::Buffer<float> uniform_buffer(
      "My uniform buffer", {1.0}, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst, webgpu);

  float time = 0.5f;
  webgpu.queue.writeBuffer(uniform_buffer.wgpu_buffer, 0, &time, sizeof(float));

  pipeline.binding.buffer = uniform_buffer.wgpu_buffer;

  pipeline.add_vertex_buffer(vertex_buffer.wgpu_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32, 1, 2 * sizeof(float));

  pipeline.finalize();

  while (lab::tick()) {
    webgpu.queue.writeBuffer(uniform_buffer.wgpu_buffer, 0, &time, sizeof(float));
    pipeline.render_frame(surface, {vertex_count, 1});
    time += 0.01f;
  }
}
