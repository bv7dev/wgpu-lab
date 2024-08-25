#include <lab>

struct MyVertex {
  float x, y;
  float u, v;
};

struct MyInstance {
  float x, y;
};

struct MyPixel {
  uint8_t r, g, b, a;
};

struct MyUniform {
  float ratio[2];
  float time;
  float scale;
};

int main() {
  lab::Window window("Dynamic Instances Demo", 640, 400);

  lab::Webgpu webgpu("My Instance");
  lab::Surface surface(window, webgpu);

  lab::Shader shader("My Shader", "shaders/sample_dynamic_instances.wgsl");
  lab::Pipeline pipeline(shader, webgpu);

  lab::Texture texture(webgpu, wgpu::TextureFormat::RGBA8Unorm, 256, 256);

  // ---------------------------------------------------------------------------
  // create procedural texture and upload to gpu
  std::vector<MyPixel> pixel_data;
  pixel_data.reserve(texture.width() * texture.height());

  auto u8 = [](float s) { return static_cast<uint8_t>(s); };
  auto transform = [&u8](int v, float s = 1.f) {
    return u8(s * (20.f + u8(sinf(static_cast<float>(v) / 255.f * 20.f) + 1.f) * 40.f));
  };

  for (int y = 0; y < texture.height(); ++y) {
    for (int x = 0; x < texture.width(); ++x) {
      pixel_data.emplace_back(transform(x + y, .6f), transform(x, .3f), transform(y, .2f), u8(255));
    }
  }

  texture.to_device(pixel_data);
  pipeline.add_texture(texture, 1, wgpu::ShaderStage::Fragment);

  // ---------------------------------------------------------------------------
  // 3 vertices - equilateral triangle   x    y    u    v
  std::vector<MyVertex> vertex_data = {{0.f, 1.f, 0.f, 0.f},
                                       {-sqrtf(3.f) / 2.f, -3.f / 6.f, 0.f, 1.f},
                                       {+sqrtf(3.f) / 2.f, -3.f / 6.f, 1.f, 1.f}};

  lab::Buffer<MyVertex> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  // 5 (triangle-)model instances             x     y      x     y
  std::vector<MyInstance> instance_data = {
      {0.f, 0.f}, {.5f, .5f}, {-.2f, .4f}, {-.7f, -.2f}, {.5f, -.6f},
  };
  lab::Buffer<MyInstance> instance_buffer("My instance buffer", instance_data, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0); // pos
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 1); // uv

  pipeline.add_vertex_buffer(instance_buffer, wgpu::VertexStepMode::Instance);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 2); // inst_pos

  MyUniform uniforms{.ratio = {window.ratio(), 1.0f}, .time = 0.0f, .scale = 0.2f};
  lab::Buffer<MyUniform> uniform_buffer("My uniform buffer", {uniforms},
                                        wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
                                        webgpu);

  pipeline.add_uniform_buffer(uniform_buffer, 0,
                              wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);

  pipeline.finalize();

  while (lab::tick(webgpu)) {
    uniform_buffer.write(uniforms);

    pipeline.render_frame(surface, vertex_data.size(), instance_data.size());

    uniforms.ratio[0] = window.ratio();
    uniforms.time = lab::elapsed_seconds();

    lab::sleep(20ms);
  }
}
