#include "common.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

using enum wgpu::VertexFormat;

// Each test renders into a texture and compares pixels that were read back.

static bool all_pixels_are(const std::vector<Pixel>& pixels, Pixel expected) {
  return std::ranges::all_of(pixels, [&](const Pixel& pixel) { return pixel == expected; });
}

// a pipeline that fills the whole target with the given color and needs no buffers
static lab::Pipeline make_fill_pipeline(lab::Gpu& gpu, const lab::Texture& target, const char* wgsl_color) {
  auto shader = lab::Shader::from_source(
      gpu, fullscreen_vertex_shader +
               std::format("@fragment fn fs_main() -> @location(0) vec4f {{ return {}; }}", wgsl_color));
  return lab::Pipeline(gpu, shader, {.target = target});
}

TEST_CASE("render: a triangle is drawn over the clear color") {
  lab::Gpu gpu;
  lab::Texture target = make_target(gpu);

  // the same triangle as samples/shaders/test1.wgsl
  auto shader = lab::Shader::from_source(gpu, R"(
    @vertex fn vs_main(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {
      var positions = array<vec2f, 3>(vec2f(-0.5, -0.5), vec2f(0.5, -0.5), vec2f(0.0, 0.5));
      return vec4f(positions[i], 0.0, 1.0);
    }
    @fragment fn fs_main() -> @location(0) vec4f {
      return vec4f(0.0, 0.4, 1.0, 1.0);
    }
  )");
  lab::Pipeline pipeline(gpu, shader, {.target = target});

  {
    lab::Frame frame(gpu);
    lab::RenderPass pass(frame, target, {.clear = clear_color});
    pass.draw(pipeline, 3);
  }
  auto pixels = target.read<Pixel>();

  CHECK(pixels[center] == Pixel{0, 102, 255, 255});
  CHECK(pixels[corner] == clear_pixel);
  CHECK(gpu.errors().empty());
}

TEST_CASE("render: vertex buffer attributes reach the shader") {
  lab::Gpu gpu;
  lab::Texture target = make_target(gpu);

  auto shader = lab::Shader::from_source(gpu, R"(
    struct VsOutput {
      @builtin(position) position: vec4f,
      @location(0) color: vec3f,
    };
    @vertex fn vs_main(@location(0) position: vec2f, @location(1) color: vec3f) -> VsOutput {
      return VsOutput(vec4f(position, 0.0, 1.0), color);
    }
    @fragment fn fs_main(in: VsOutput) -> @location(0) vec4f {
      return vec4f(in.color, 1.0);
    }
  )");

  struct Vertex {
    float pos[2];
    float color[3];
  };
  std::vector<Vertex> vertex_data = {
      {.pos = {-0.5f, -0.5f}, .color = {1.0f, 1.0f, 0.0f}},
      {.pos = {+0.5f, -0.5f}, .color = {1.0f, 1.0f, 0.0f}},
      {.pos = {+0.0f, +0.5f}, .color = {1.0f, 1.0f, 0.0f}},
  };
  lab::Buffer vertices(gpu, vertex_data);
  lab::Pipeline pipeline(gpu, shader,
                         {.vertex_buffers = {lab::vertex<Vertex>({Float32x2, Float32x3})}, .target = target});

  {
    lab::Frame frame(gpu);
    lab::RenderPass pass(frame, target, {.clear = clear_color});
    pass.draw(pipeline, {.vertex_buffers = {vertices}}); // 3 vertices, taken from the buffer
  }
  auto pixels = target.read<Pixel>();

  CHECK(pixels[center] == Pixel{255, 255, 0, 255});
  CHECK(pixels[corner] == clear_pixel);
  CHECK(gpu.errors().empty());
}

TEST_CASE("render: a uniform buffer is bound at its binding index") {
  lab::Gpu gpu;
  lab::Texture target = make_target(gpu);

  auto shader = lab::Shader::from_source(gpu, fullscreen_vertex_shader + R"(
    @group(0) @binding(2) var<uniform> color: vec4f;
    @fragment fn fs_main() -> @location(0) vec4f {
      return color;
    }
  )");
  lab::Pipeline pipeline(gpu, shader, {.target = target});

  struct Color {
    float r, g, b, a;
  };
  lab::Buffer<Color> uniforms(gpu, {{0.0f, 1.0f, 0.0f, 1.0f}}, wgpu::BufferUsage::Uniform);
  lab::Draw draw{.bind_groups = {pipeline.bind_group(0, {{2, uniforms}})}, .count = 3};

  {
    lab::Frame frame(gpu);
    lab::RenderPass pass(frame, target);
    pass.draw(pipeline, draw);
  }
  CHECK(all_pixels_are(target.read<Pixel>(), {0, 255, 0, 255}));

  // the same draw again after the uniform has changed
  uniforms.write({0.0f, 0.0f, 1.0f, 1.0f});
  {
    lab::Frame frame(gpu);
    lab::RenderPass pass(frame, target);
    pass.draw(pipeline, draw);
  }
  CHECK(all_pixels_are(target.read<Pixel>(), {0, 0, 255, 255}));
  CHECK(gpu.errors().empty());
}

TEST_CASE("render: indexed and instanced drawing, with counts taken from the buffers") {
  lab::Gpu gpu;
  lab::Texture target = make_target(gpu);

  // a quad per instance: the first instance covers the left half, the second one the right half
  auto shader = lab::Shader::from_source(gpu, R"(
    struct VsOutput {
      @builtin(position) position: vec4f,
      @location(0) color: vec3f,
    };
    @vertex fn vs_main(@location(0) corner: vec2f, @location(1) offset: f32, @location(2) color: vec3f) -> VsOutput {
      return VsOutput(vec4f(corner.x * 0.5 + offset, corner.y, 0.0, 1.0), color);
    }
    @fragment fn fs_main(in: VsOutput) -> @location(0) vec4f {
      return vec4f(in.color, 1.0);
    }
  )");

  struct Corner {
    float x, y;
  };
  struct Instance {
    float offset;
    float color[3];
  };
  lab::Buffer<Corner> corners(gpu, {{-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}});
  lab::Buffer<uint16_t> indices(gpu, {0, 1, 2, 2, 3, 0}, wgpu::BufferUsage::Index);
  lab::Buffer<Instance> instances(gpu, {{-0.5f, {0.0f, 0.0f, 1.0f}}, {+0.5f, {0.0f, 1.0f, 1.0f}}});

  lab::Pipeline pipeline(gpu, shader,
                         {.vertex_buffers = {lab::vertex<Corner>({Float32x2}),               // location 0
                                             lab::instance<Instance>({Float32, Float32x3})}, // locations 1 and 2
                          .target = target});

  {
    lab::Frame frame(gpu);
    lab::RenderPass pass(frame, target, {.clear = clear_color});
    pass.draw(pipeline, {.vertex_buffers = {corners, instances}, .index_buffer = indices}); // 6 indices, 2 instances
  }
  auto pixels = target.read<Pixel>();

  CHECK(pixels[at(target_size / 4, target_size / 2)] == Pixel{0, 0, 255, 255});       // left half
  CHECK(pixels[at(target_size * 3 / 4, target_size / 2)] == Pixel{0, 255, 255, 255}); // right half
  CHECK(gpu.errors().empty());
}

TEST_CASE("render: two bind groups, one of them bound to a part of a buffer") {
  lab::Gpu gpu;
  lab::Texture target = make_target(gpu);

  auto shader = lab::Shader::from_source(gpu, fullscreen_vertex_shader + R"(
    @group(0) @binding(0) var<uniform> red: f32;
    @group(1) @binding(0) var<uniform> green_blue: vec2f;
    @fragment fn fs_main() -> @location(0) vec4f {
      return vec4f(red, green_blue, 1.0);
    }
  )");
  lab::Pipeline pipeline(gpu, shader, {.target = target});

  lab::Buffer<float> red(gpu, {1.0f}, wgpu::BufferUsage::Uniform);

  // two blocks in one buffer: a uniform binding has to start at a multiple of 256 bytes
  struct alignas(256) Block {
    float green, blue;
  };
  lab::Buffer<Block> blocks(gpu, {{0.0f, 0.0f}, {0.0f, 1.0f}}, wgpu::BufferUsage::Uniform);

  lab::Draw draw{
      .bind_groups = {pipeline.bind_group(0, {{0, red}}),
                      pipeline.bind_group(1, {{0, blocks.handle(), sizeof(Block), sizeof(Block)}})}, // the second block
      .count = 3,
  };
  {
    lab::Frame frame(gpu);
    lab::RenderPass pass(frame, target);
    pass.draw(pipeline, draw);
  }
  CHECK(all_pixels_are(target.read<Pixel>(), {255, 0, 255, 255}));
  CHECK(gpu.errors().empty());
}

TEST_CASE("render: several pipelines in one pass, several passes in one frame") {
  lab::Gpu gpu;
  lab::Texture target = make_target(gpu);
  lab::Pipeline green = make_fill_pipeline(gpu, target, "vec4f(0.0, 1.0, 0.0, 1.0)");
  lab::Pipeline half_blue = make_fill_pipeline(gpu, target, "vec4f(0.0, 0.0, 1.0, 0.5)");

  SUBCASE("a second pipeline draws over the first, blended") {
    {
      lab::Frame frame(gpu);
      lab::RenderPass pass(frame, target, {.clear = clear_color});
      pass.draw(green, 3);
      pass.draw(half_blue, 3);
    }
    Pixel mixed = target.read<Pixel>()[center];
    CHECK(mixed.r == 0);
    CHECK(mixed.g == doctest::Approx(128).epsilon(0.02));
    CHECK(mixed.b == doctest::Approx(128).epsilon(0.02));
  }

  SUBCASE("a second pass can keep what the first one drew") {
    {
      lab::Frame frame(gpu);
      {
        lab::RenderPass first(frame, target, {.clear = clear_color});
        first.draw(green, 3);
      }
      lab::RenderPass second(frame, target, {.clear = std::nullopt});
    }
    CHECK(all_pixels_are(target.read<Pixel>(), {0, 255, 0, 255}));
  }

  SUBCASE("or clear it again") {
    {
      lab::Frame frame(gpu);
      lab::RenderPass first(frame, target);
      first.draw(green, 3);
      first.end();
      lab::RenderPass second(frame, target, {.clear = clear_color});
    }
    CHECK(all_pixels_are(target.read<Pixel>(), clear_pixel));
  }
  CHECK(gpu.errors().empty());
}

TEST_CASE("Texture: written pixels can be read back and sampled") {
  lab::Gpu gpu;

  // 2x2 pixels: red, green / blue, white
  lab::Texture image(gpu, wgpu::TextureFormat::RGBA8Unorm, 2, 2);
  const std::vector<Pixel> image_pixels = {{255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {255, 255, 255, 255}};
  image.write(image_pixels);

  CHECK(image.read<Pixel>() == image_pixels);

  SUBCASE("a pixel type of the wrong size is an error") {
    CHECK_THROWS_WITH_AS(image.read<uint16_t>(), doctest::Contains("has 4 bytes"), lab::Error);
    CHECK_THROWS_WITH_AS(image.write(std::vector<Pixel>(3)), doctest::Contains("2x2 = 4 pixels"), lab::Error);
  }

  SUBCASE("a shader reads it, stretched over the target") {
    lab::Texture target = make_target(gpu);
    auto shader = lab::Shader::from_source(gpu, fullscreen_vertex_shader + R"(
      @group(0) @binding(0) var image: texture_2d<f32>;
      @fragment fn fs_main(@builtin(position) position: vec4f) -> @location(0) vec4f {
        return textureLoad(image, vec2u(position.xy) / 32, 0);
      }
    )");
    lab::Pipeline pipeline(gpu, shader, {.target = target, .blend = std::nullopt});
    {
      lab::Frame frame(gpu);
      lab::RenderPass pass(frame, target);
      pass.draw(pipeline, {.bind_groups = {pipeline.bind_group(0, {{0, image}})}, .count = 3});
    }
    auto pixels = target.read<Pixel>();

    CHECK(pixels[at(16, 16)] == image_pixels[0]); // top left
    CHECK(pixels[at(48, 16)] == image_pixels[1]); // top right
    CHECK(pixels[at(16, 48)] == image_pixels[2]); // bottom left
    CHECK(pixels[at(48, 48)] == image_pixels[3]); // bottom right
  }
  CHECK(gpu.errors().empty());
}

TEST_CASE("Texture: save_png writes a PNG file") {
  lab::Gpu gpu;
  lab::Texture target = make_target(gpu);
  lab::Pipeline green = make_fill_pipeline(gpu, target, "vec4f(0.0, 1.0, 0.0, 1.0)");
  {
    lab::Frame frame(gpu);
    lab::RenderPass pass(frame, target);
    pass.draw(green, 3);
  }

  const auto path = std::filesystem::temp_directory_path() / "wgpu-lab-test-save.png";
  std::filesystem::remove(path);
  target.save_png(path);

  std::ifstream file(path, std::ios::binary);
  REQUIRE(file);
  char signature[8] = {};
  file.read(signature, sizeof(signature));
  CHECK(std::string_view(signature, 8) == "\x89PNG\r\n\x1a\n");
  // stored without compression: a bit more than 4 bytes per pixel
  CHECK(std::filesystem::file_size(path) > target_size * target_size * 4);
  std::filesystem::remove(path);
}
