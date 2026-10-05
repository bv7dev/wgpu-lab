// 05: a texture is filled with pixels and sampled by the fragment shader

#include <lab>

#include <cmath>
#include <vector>

using enum wgpu::VertexFormat;

struct MyVertex {
  float x, y;
  float u, v;
};

struct MyUniforms {
  float ratio[2];
  float time;
  float scale;
};

// one pixel of an RGBA8Unorm texture
struct MyPixel {
  uint8_t r, g, b, a;
};

int main() {
  lab::Gpu gpu;
  lab::Window window("Texture", 640, 400);
  lab::Surface surface(gpu, window);

  // ---------------------------------------------------------------------------
  // create a procedural texture and upload it to the gpu
  lab::Texture texture(gpu, wgpu::TextureFormat::RGBA8Unorm, 256, 256);

  auto wave = [](int v, float s = 1.f) {
    return uint8_t(s * (20.f + uint8_t(std::sin(float(v) / 255.f * 20.f) + 1.f) * 40.f));
  };

  std::vector<MyPixel> pixel_data;
  pixel_data.reserve(256 * 256);
  for (int y = 0; y < 256; ++y) {
    for (int x = 0; x < 256; ++x) {
      pixel_data.push_back({wave(x + y, .6f), wave(x, .3f), wave(y, .2f), 255});
    }
  }
  texture.write(pixel_data);

  // ---------------------------------------------------------------------------
  // an equilateral triangle with texture coordinates
  //                                     x                       y      u    v
  std::vector<MyVertex> vertex_data = {
      {0.f, 1.f, 0.f, 0.f}, {-std::sqrt(3.f) / 2.f, -0.5f, 0.f, 1.f}, {+std::sqrt(3.f) / 2.f, -0.5f, 1.f, 1.f}};
  lab::Buffer vertices(gpu, vertex_data);

  MyUniforms uniforms{.ratio = {1.0f / surface.aspect(), 1.0f}, .time = 0.0f, .scale = 0.4f};
  lab::Buffer<MyUniforms> uniform_buffer(gpu, {uniforms}, wgpu::BufferUsage::Uniform);

  lab::Shader shader(gpu, "shaders/05_texture.wgsl");
  lab::Pipeline pipeline(gpu, shader,
                         {.vertex_buffers = {lab::vertex<MyVertex>({Float32x2, Float32x2})}, // position, uv
                          .target = surface});

  // the bind group connects the resources the shader declares:
  // `@binding(0) var<uniform> uniforms`, `@binding(1) var gradientTexture` and `@binding(2) var gradientSampler`
  lab::Draw triangle{
      .vertex_buffers = {vertices},
      .bind_groups = {pipeline.bind_group(0, {{0, uniform_buffer}, {1, texture}, {2, lab::sampler(gpu)}})},
  };

  while (lab::tick()) {
    uniforms.ratio[0] = 1.0f / surface.aspect();
    uniforms.time = lab::elapsed_seconds();
    uniform_buffer.write(uniforms);

    pipeline.render_frame(surface, triangle);
  }
}
