#include "offscreen.h"

#include <algorithm>

using offscreen::Pixel;

// a triangle that covers the whole render target, see
// https://webgpufundamentals.org/webgpu/lessons/webgpu-large-triangle-to-cover-clip-space.html
static const char* const fullscreen_vertex_shader = R"(
  @vertex fn vs_main(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {
    var positions = array<vec2f, 3>(vec2f(-1.0, -1.0), vec2f(3.0, -1.0), vec2f(-1.0, 3.0));
    return vec4f(positions[i], 0.0, 1.0);
  }
)";

TEST_CASE("render: a triangle is drawn over the clear color") {
  lab::Webgpu webgpu("test");
  REQUIRE(webgpu.device);
  webgpu.surface_format = offscreen::format;

  // the same shader as samples/shaders/test1.wgsl
  lab::Shader shader("triangle");
  shader.source = R"(
    @vertex fn vs_main(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {
      var positions = array<vec2f, 3>(vec2f(-0.5, -0.5), vec2f(0.5, -0.5), vec2f(0.0, 0.5));
      return vec4f(positions[i], 0.0, 1.0);
    }
    @fragment fn fs_main() -> @location(0) vec4f {
      return vec4f(0.0, 0.4, 1.0, 1.0);
    }
  )";
  lab::Pipeline pipeline(shader, webgpu, true);

  auto pixels = offscreen::render(pipeline, 3);

  CHECK(pixels[offscreen::center] == Pixel{0, 102, 255, 255});
  CHECK(pixels[offscreen::corner] == offscreen::clear_color);
}

TEST_CASE("render: vertex buffer attributes reach the shader") {
  lab::Webgpu webgpu("test");
  REQUIRE(webgpu.device);
  webgpu.surface_format = offscreen::format;

  lab::Shader shader("colored vertices");
  shader.source = R"(
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
  )";
  lab::Pipeline pipeline(shader, webgpu);

  struct Vertex {
    float pos[2];
    float color[3];
  };
  std::vector<Vertex> vertices = {
      {.pos = {-0.5f, -0.5f}, .color = {1.0f, 1.0f, 0.0f}},
      {.pos = {+0.5f, -0.5f}, .color = {1.0f, 1.0f, 0.0f}},
      {.pos = {+0.0f, +0.5f}, .color = {1.0f, 1.0f, 0.0f}},
  };
  lab::Buffer vertex_buffer("vertices", vertices, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 0);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x3, 1);
  pipeline.finalize();

  auto pixels = offscreen::render(pipeline, 3);

  CHECK(pixels[offscreen::center] == Pixel{255, 255, 0, 255});
  CHECK(pixels[offscreen::corner] == offscreen::clear_color);
}

TEST_CASE("render: a uniform buffer is bound at the binding index it was added with") {
  lab::Webgpu webgpu("test");
  REQUIRE(webgpu.device);
  webgpu.surface_format = offscreen::format;

  lab::Shader shader("uniform color");
  shader.source = std::string(fullscreen_vertex_shader) + R"(
    @group(0) @binding(2) var<uniform> color: vec4f;
    @fragment fn fs_main() -> @location(0) vec4f {
      return color;
    }
  )";
  lab::Pipeline pipeline(shader, webgpu);

  struct Color {
    float r, g, b, a;
  };
  lab::Buffer<Color> uniform_buffer("color", {{0.0f, 1.0f, 0.0f, 1.0f}}, wgpu::BufferUsage::Uniform, webgpu);
  pipeline.add_uniform_buffer(uniform_buffer, 2, wgpu::ShaderStage::Fragment);
  pipeline.finalize();

  auto pixels = offscreen::render(pipeline, 3);

  CHECK(std::ranges::all_of(pixels, [](const Pixel& p) { return p == Pixel{0, 255, 0, 255}; }));
}

TEST_CASE("render: indexed and instanced drawing") {
  lab::Webgpu webgpu("test");
  REQUIRE(webgpu.device);
  webgpu.surface_format = offscreen::format;

  // a quad per instance: the first instance covers the left half, the second one the right half
  lab::Shader shader("instanced quads");
  shader.source = R"(
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
  )";
  lab::Pipeline pipeline(shader, webgpu);

  struct Corner {
    float x, y;
  };
  struct Instance {
    float offset;
    float color[3];
  };
  std::vector<Corner> corners = {{-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}};
  std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};
  std::vector<Instance> instances = {{-0.5f, {0.0f, 0.0f, 1.0f}}, {+0.5f, {0.0f, 1.0f, 1.0f}}};

  lab::Buffer corner_buffer("corners", corners, webgpu);
  lab::Buffer index_buffer("indices", indices, wgpu::BufferUsage::Index, webgpu);
  lab::Buffer instance_buffer("instances", instances, webgpu);

  pipeline.add_vertex_buffer(corner_buffer);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 0);
  pipeline.add_index_buffer(index_buffer, wgpu::IndexFormat::Uint16);
  pipeline.add_vertex_buffer(instance_buffer, wgpu::VertexStepMode::Instance);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32, 1);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x3, 2);
  pipeline.finalize();

  auto pixels = offscreen::render(pipeline, 6, 2);

  const size_t row = (offscreen::size / 2) * offscreen::size;
  CHECK(pixels[row + offscreen::size / 4] == Pixel{0, 0, 255, 255});       // left half
  CHECK(pixels[row + offscreen::size * 3 / 4] == Pixel{0, 255, 255, 255}); // right half
}
