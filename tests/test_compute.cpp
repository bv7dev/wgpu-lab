#include "common.h"

#include <numeric>

static const char* const square_shader = R"(
  @group(0) @binding(0) var<storage, read_write> data: array<f32>;

  @compute @workgroup_size(64)
  fn main(@builtin(global_invocation_id) id: vec3u) {
    if (id.x < arrayLength(&data)) {
      data[id.x] = data[id.x] * data[id.x];
    }
  }
)";

TEST_CASE("compute: a shader works on a storage buffer") {
  lab::Gpu gpu;
  std::vector<float> input(1000);
  std::iota(input.begin(), input.end(), 0.0f);
  lab::Buffer data(gpu, input, wgpu::BufferUsage::Storage);

  auto shader = lab::Shader::from_source(gpu, square_shader, "square");
  lab::ComputePipeline square(gpu, shader);

  // 1000 elements in workgroups of 64: 16 groups, the shader ignores what is beyond the array
  square.run({square.bind_group(0, {{0, data}})}, (data.size() + 63) / 64);

  std::vector<float> result = data.read();
  CHECK(result[3] == 9.0f);
  CHECK(result[999] == 999.0f * 999.0f);
  CHECK(gpu.errors().empty());
}

TEST_CASE("compute: a compute pass and a render pass in one frame") {
  lab::Gpu gpu;
  lab::Texture target = make_target(gpu);

  // the compute shader writes the color that the render pass then fills the target with
  lab::Buffer<float> color(gpu, 4, wgpu::BufferUsage::Storage);
  auto compute_shader = lab::Shader::from_source(gpu, R"(
    @group(0) @binding(0) var<storage, read_write> color: array<f32, 4>;
    @compute @workgroup_size(1) fn main() {
      color = array<f32, 4>(0.0, 1.0, 1.0, 1.0);
    }
  )");
  lab::ComputePipeline compute(gpu, compute_shader);

  auto render_shader = lab::Shader::from_source(gpu, fullscreen_vertex_shader + R"(
    @group(0) @binding(0) var<storage, read> color: array<f32, 4>;
    @fragment fn fs_main() -> @location(0) vec4f {
      return vec4f(color[0], color[1], color[2], color[3]);
    }
  )");
  lab::Pipeline pipeline(gpu, render_shader, {.target = target});

  {
    lab::Frame frame(gpu);
    {
      lab::ComputePass pass(frame);
      pass.dispatch(compute, {compute.bind_group(0, {{0, color}})}, 1);
    }
    lab::RenderPass pass(frame, target);
    pass.draw(pipeline, {.bind_groups = {pipeline.bind_group(0, {{0, color}})}, .count = 3});
  }

  CHECK(target.read<Pixel>()[center] == Pixel{0, 255, 255, 255});
  CHECK(gpu.errors().empty());
}

TEST_CASE("compute: a shader that does not fit is an error where the pipeline is created") {
  lab::Gpu gpu;
  auto shader = lab::Shader::from_source(gpu, "@fragment fn fs_main() -> @location(0) vec4f { return vec4f(1.0); }",
                                         "no compute stage");

  CHECK_THROWS_WITH_AS(lab::ComputePipeline(gpu, shader), doctest::Contains("compute pipeline(no compute stage)"),
                       lab::Error);
  CHECK(gpu.errors().empty());
}
