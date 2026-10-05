#include "common.h"

using enum wgpu::VertexFormat;

static const std::string blue_fragment_shader = R"(
  @fragment fn fs_main() -> @location(0) vec4f {
    return vec4f(0.0, 0.0, 1.0, 1.0);
  }
)";

TEST_CASE("Shader: a missing file is reported with the places that were searched") {
  lab::Gpu gpu;
  CHECK_THROWS_WITH_AS(lab::Shader(gpu, "does/not/exist.wgsl"), doctest::Contains("does/not/exist.wgsl"), lab::Error);
}

TEST_CASE("Shader: a compile error names the shader and the line") {
  lab::Gpu gpu;
  const char* broken = "@fragment fn fs_main() -> @location(0) vec4f {\n  return vec4f(1.0)\n}"; // missing ';'

  try {
    lab::Shader::from_source(gpu, broken, "broken shader");
    FAIL("the shader should not have compiled");
  } catch (const lab::Error& error) {
    const std::string what = error.what();
    CHECK(what.starts_with("broken shader: does not compile"));
    CHECK(what.find(":3:1") != std::string::npos); // line 3, column 1: where the ';' is missed
  }
  CHECK(gpu.errors().empty()); // reported as an exception, not as a stray GPU error
}

TEST_CASE("Pipeline: a vertex layout that does not provide what the shader reads is an error") {
  lab::Gpu gpu;
  auto shader = lab::Shader::from_source(gpu, R"(
    @vertex fn vs_main(@location(0) position: vec2f) -> @builtin(position) vec4f {
      return vec4f(position, 0.0, 1.0);
    }
  )" + blue_fragment_shader,
                                         "needs a position");

  // no vertex buffers declared
  CHECK_THROWS_WITH_AS(lab::Pipeline(gpu, shader, {.target = target_format}),
                       doctest::Contains("pipeline(needs a position)"), lab::Error);
  CHECK(gpu.errors().empty());
}

TEST_CASE("Pipeline: bind_group reports bindings that do not match the shader") {
  lab::Gpu gpu;
  auto shader = lab::Shader::from_source(gpu, fullscreen_vertex_shader + R"(
    @group(0) @binding(2) var<uniform> color: vec4f;
    @fragment fn fs_main() -> @location(0) vec4f {
      return color;
    }
  )");
  lab::Pipeline pipeline(gpu, shader, {.target = target_format});
  lab::Buffer<float> color(gpu, {0.0f, 1.0f, 0.0f, 1.0f}, wgpu::BufferUsage::Uniform);

  CHECK_NOTHROW(pipeline.bind_group(0, {{2, color}}));
  CHECK_THROWS_AS(pipeline.bind_group(0, {{0, color}}), lab::Error); // the shader has no binding 0
  CHECK_THROWS_AS(pipeline.bind_group(1, {{2, color}}), lab::Error); // the shader has no group 1
  CHECK(gpu.errors().empty());
}

TEST_CASE("RenderPass: draw calls that do not fit the pipeline are errors") {
  lab::Gpu gpu;
  lab::Texture target = make_target(gpu);
  auto shader = lab::Shader::from_source(gpu, R"(
    @vertex fn vs_main(@location(0) position: vec2f) -> @builtin(position) vec4f {
      return vec4f(position, 0.0, 1.0);
    }
  )" + blue_fragment_shader);

  struct Position {
    float x, y;
  };
  lab::Pipeline pipeline(gpu, shader, {.vertex_buffers = {lab::vertex<Position>({Float32x2})}, .target = target});
  lab::Buffer<Position> positions(gpu, {{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}});
  lab::Buffer<float> floats(gpu, {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f});

  lab::Frame frame(gpu);
  lab::RenderPass pass(frame, target);

  CHECK_NOTHROW(pass.draw(pipeline, {.vertex_buffers = {positions}}));

  SUBCASE("a missing vertex buffer") {
    CHECK_THROWS_WITH_AS(pass.draw(pipeline, 3), doctest::Contains("declares 1 vertex buffer"), lab::Error);
  }
  SUBCASE("a vertex buffer with another element size") {
    CHECK_THROWS_WITH_AS(pass.draw(pipeline, {.vertex_buffers = {floats}}), doctest::Contains("elements of 4 bytes"),
                         lab::Error);
  }
  SUBCASE("a pipeline that was built for another target format") {
    lab::Pipeline other(
        gpu, shader,
        {.vertex_buffers = {lab::vertex<Position>({Float32x2})}, .target = wgpu::TextureFormat::BGRA8Unorm});
    CHECK_THROWS_WITH_AS(pass.draw(other, {.vertex_buffers = {positions}}), doctest::Contains("BGRA8Unorm"),
                         lab::Error);
  }
}

TEST_CASE("Gpu: errors reported by WebGPU are thrown by poll") {
  // a buffer that is both mappable for reading and a vertex buffer is not allowed
  const wgpu::BufferDescriptor invalid{
      .usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::Vertex,
      .size = 16,
  };

  SUBCASE("by default") {
    lab::Gpu gpu;
    lab::set_log_level(lab::LogLevel::off); // the error is expected, keep the test output clean
    gpu.device().CreateBuffer(&invalid);
    lab::set_log_level(lab::LogLevel::info);

    CHECK_THROWS_WITH_AS(gpu.poll(), doctest::Contains("the GPU reported an error"), lab::Error);
    CHECK_NOTHROW(gpu.poll()); // each error is thrown once
  }
  SUBCASE("or only collected, with throw_on_error = false") {
    lab::Gpu gpu({.throw_on_error = false});
    lab::set_log_level(lab::LogLevel::off);
    gpu.device().CreateBuffer(&invalid);
    lab::set_log_level(lab::LogLevel::info);

    CHECK_NOTHROW(gpu.poll());
    CHECK(gpu.errors().size() == 1);
  }
}
