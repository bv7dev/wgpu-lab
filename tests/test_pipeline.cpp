#include <doctest/doctest.h>

#include <lab>

TEST_CASE("Pipeline: add_uniform_buffer uses the given binding index") {
  lab::Webgpu webgpu("test");
  REQUIRE(webgpu.device);

  lab::Shader shader("unused");
  lab::Pipeline pipeline(shader, webgpu);
  lab::Buffer<float> uniforms("uniforms", {1.0f}, wgpu::BufferUsage::Uniform, webgpu);

  pipeline.add_uniform_buffer(uniforms, 3, wgpu::ShaderStage::Fragment);

  CHECK(pipeline.bindGroupLayoutEntries.back().binding == 3);
  CHECK(pipeline.bindGroupEntries.back().binding == 3);
}

TEST_CASE("Pipeline: vertex attributes are laid out one after another") {
  lab::Webgpu webgpu("test");
  REQUIRE(webgpu.device);

  lab::Shader shader("unused");
  lab::Pipeline pipeline(shader, webgpu);
  lab::Buffer<float> vertices("vertices", {0.0f, 0.0f, 0.0f, 0.0f, 0.0f}, webgpu);

  pipeline.add_vertex_buffer(vertices);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 0);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x3, 1);

  const auto& attributes = pipeline.vb_configs.back().vertexAttributes;
  REQUIRE(attributes.size() == 2);
  CHECK(attributes[0].offset == 0);
  CHECK(attributes[1].offset == 8);
  CHECK(lab::vertex_attributes_stride(attributes) == 20);
}

TEST_CASE("Shader: a missing file is reported with its path") {
  CHECK_THROWS_WITH_AS(lab::Shader("missing", "does/not/exist.wgsl"), doctest::Contains("does/not/exist.wgsl"),
                       std::runtime_error);
}
