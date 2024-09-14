#include <lab>

struct MyVertex {
  float x, y;
  float u, v;
};

struct MyUniforms {
  float ratio[2];
  float time;
  float scale;
};

struct MyPixelFormat {
  uint8_t r, g, b, a;
};

int main() {
  lab::Window window("Texture Demo", 640, 400);

  lab::Webgpu webgpu("My Instance");
  lab::Surface surface(window, webgpu);

  lab::Shader shader("My Shader", "shaders/sample_texture.wgsl");
  lab::Pipeline pipeline(shader, webgpu);

  lab::Texture texture(webgpu, wgpu::TextureFormat::RGBA8Unorm, 256, 256);

  // ---------------------------------------------------------------------------
  // create procedural texture and upload to gpu
  std::vector<MyPixelFormat> pixel_data;
  pixel_data.reserve(texture.width() * texture.height());

  auto transform = [](int v, float s = 1.f) {
    return uint8_t(s * (20.f + uint8_t(sinf(float(v) / 255.f * 20.f) + 1.f) * 40.f));
  };

  for (int y = 0; y < texture.height(); ++y) {
    for (int x = 0; x < texture.width(); ++x) {
      pixel_data.emplace_back(transform(x + y, .6f), transform(x, .3f), transform(y, .2f), uint8_t(255u));
    }
  }

  texture.to_device(pixel_data);
  pipeline.add_texture(texture, 1, wgpu::ShaderStage::Fragment);

  // ---------------------------------------------------------------------------
  // create vertex buffer with 3 vertices which form an equilateral triangle
  //    x    y    u    v                 x           y       u    v
  std::vector<MyVertex> vertex_data = {
      {0.f, 1.f, 0.f, 0.f}, {-sqrtf(3.f) / 2.f, -3.f / 6.f, 0.f, 1.f}, {+sqrtf(3.f) / 2.f, -3.f / 6.f, 1.f, 1.f}};

  lab::Buffer<MyVertex> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 0); // pos
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 1); // uv

  MyUniforms uniforms{.ratio = {window.ratio(), 1.0f}, .time = 0.0f, .scale = 0.4f};
  lab::Buffer<MyUniforms> uniform_buffer("My uniform buffer", {uniforms},
                                         wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst, webgpu);

  pipeline.add_uniform_buffer(uniform_buffer, 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);

  pipeline.finalize();

  while (lab::tick()) {
    uniform_buffer.write(uniforms);

    pipeline.render_frame(surface, vertex_data.size(), 1);

    uniforms.ratio[0] = window.ratio();
    uniforms.time = lab::elapsed_seconds();

    lab::sleep(20ms);
  }
}
