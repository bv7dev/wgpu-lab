#include <lab>

#include <objects/lab_texture.h>

struct MyVertex {
  float x, y;
  float u, v;
};

struct MyUniforms {
  float ratio[2];
  float time;
  float scale;
};

int main() {
  lab::Window window("Vertex Buffer & Uniforms Demo", 640, 400);

  lab::Webgpu webgpu("My Instance");
  lab::Surface surface(window, webgpu);

  lab::Shader shader("My Shader", "shaders/learn_textures.wgsl");
  lab::Pipeline pipeline(shader, webgpu);

  lab::Texture texture({256, 256}, webgpu);

  // Create image data
  std::vector<uint8_t> pixels(4 * texture.width() * texture.height());
  for (uint32_t x = 0; x < texture.width(); ++x) {
    for (uint32_t y = 0; y < texture.height(); ++y) {
      uint8_t* p = &pixels[4 * (y * texture.width() + x)];
      p[0] = (uint8_t)x; // r
      p[1] = (uint8_t)y; // g
      p[2] = 128;        // b
      p[3] = 255;        // a
    }
  }

  texture.to_device(pixels);
  pipeline.add_texture(texture);

  // 3 vertices - equilateral triangle   x    y    u    v
  std::vector<MyVertex> vertex_data = {{0.f, 1.f, .5f, 0.f},
                                       {-sqrtf(3.f) / 2.f, -3.f / 6.f, 0.f, 1.f},
                                       {sqrtf(3.f) / 2.f, -3.f / 6.f, 1.f, 1.f}};
  lab::Buffer<MyVertex> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0); // pos
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 1); // uv

  MyUniforms uniforms{.ratio = {window.ratio(), 1.0f}, .time = 0.0f, .scale = 0.4f};
  lab::Buffer<MyUniforms> uniform_buffer("My uniform buffer", {uniforms},
                                         wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
                                         webgpu);

  pipeline.add_uniform_buffer(uniform_buffer, 0,
                              wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
  pipeline.finalize();

  while (lab::tick()) {
    uniform_buffer.write(uniforms);

    pipeline.render_frame(surface, {(uint32_t)vertex_data.size(), 1});

    uniforms.ratio[0] = window.ratio();
    uniforms.time = lab::elapsed_seconds();

    lab::sleep(20ms);
  }
}
