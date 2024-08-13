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

  struct Buf2T {
    float val;
    float _pad[63];
  };
  lab::Buffer<Buf2T> uniform_buffer2("My uniform buffer 2", {{}, {}},
                                     wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
                                     webgpu);

  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32, 1, 2 * sizeof(float));

  // WIP: bind group layouts
  pipeline.add_bind_group_entry(uniform_buffer.wgpu_buffer, 0, 0, sizeof(float));
  pipeline.add_bind_group_layout_entry(0, wgpu::ShaderStage::Vertex,
                                       wgpu::BufferBindingType::Uniform, sizeof(float));
  pipeline.finalize_bind_group();

  pipeline.bindGroupEntries.clear();
  pipeline.bindGroupLayoutEntries.clear();

  pipeline.add_bind_group_entry(uniform_buffer2.wgpu_buffer, 0, 0, sizeof(Buf2T));
  pipeline.add_bind_group_layout_entry(0, wgpu::ShaderStage::Vertex,
                                       wgpu::BufferBindingType::Uniform, sizeof(Buf2T));

  pipeline.add_bind_group_entry(uniform_buffer2.wgpu_buffer, 1, sizeof(Buf2T), sizeof(Buf2T));
  pipeline.add_bind_group_layout_entry(1, wgpu::ShaderStage::Fragment,
                                       wgpu::BufferBindingType::Uniform, sizeof(Buf2T));

  pipeline.finalize_bind_group(" - Default Bind Group 2");

  pipeline.finalize();

  float time = 0.0f;
  Buf2T x = {0.0f};
  Buf2T y = {0.0f};

  while (lab::tick()) {
    uniform_buffer.write(time);
    uniform_buffer2.write(x);
    uniform_buffer2.write(y, 1);
    pipeline.render_frame(surface, {vertex_count, 1});
    time += 0.01f;
    x.val += 0.001f;
    y.val += 0.01f;
  }
}
