#include "common.h"

using enum wgpu::VertexFormat;

// vertex_format_size is usable at compile time
static_assert(lab::vertex_format_size(Float32x3) == 12);

TEST_CASE("vertex_format_size: components times bytes per component") {
  CHECK(lab::vertex_format_size(Uint8) == 1);
  CHECK(lab::vertex_format_size(Snorm8x2) == 2);
  CHECK(lab::vertex_format_size(Unorm8x4) == 4);
  CHECK(lab::vertex_format_size(Unorm8x4BGRA) == 4);
  CHECK(lab::vertex_format_size(Uint16) == 2);
  CHECK(lab::vertex_format_size(Float16x2) == 4);
  CHECK(lab::vertex_format_size(Sint16x4) == 8);
  CHECK(lab::vertex_format_size(Float32) == 4);
  CHECK(lab::vertex_format_size(Float32x2) == 8);
  CHECK(lab::vertex_format_size(Float32x3) == 12);
  CHECK(lab::vertex_format_size(Float32x4) == 16);
  CHECK(lab::vertex_format_size(Uint32x3) == 12);
  CHECK(lab::vertex_format_size(Sint32x4) == 16);
  CHECK(lab::vertex_format_size(Unorm10_10_10_2) == 4);
}

struct Vertex {
  float position[2];
  float color[3];
  uint8_t flags[4];
};

TEST_CASE("vertex layout: attributes follow each other") {
  lab::VertexLayout layout = lab::vertex<Vertex>({Float32x2, Float32x3, Unorm8x4});

  CHECK(layout.stride == sizeof(Vertex));
  CHECK(layout.step_mode == wgpu::VertexStepMode::Vertex);
  REQUIRE(layout.attributes.size() == 3);
  CHECK(layout.attributes[0].offset == 0);
  CHECK(layout.attributes[1].offset == 8);
  CHECK(layout.attributes[2].offset == 20);
}

TEST_CASE("vertex layout: an explicit offset moves the attributes after it") {
  // skips the color
  lab::VertexLayout layout = lab::instance<Vertex>({Float32x2, {Unorm8x4, 5, offsetof(Vertex, flags)}});

  CHECK(layout.step_mode == wgpu::VertexStepMode::Instance);
  REQUIRE(layout.attributes.size() == 2);
  CHECK(layout.attributes[1].offset == 20);
  CHECK(layout.attributes[1].location == 5);
}

TEST_CASE("vertex layout: attributes that do not fit into the vertex type are an error") {
  CHECK_THROWS_WITH_AS(lab::vertex<Vertex>({Float32x4, Float32x4}), doctest::Contains("has only 24 bytes"), lab::Error);
}
