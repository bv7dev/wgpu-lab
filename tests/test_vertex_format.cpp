#include <doctest/doctest.h>

#include <lab>

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

TEST_CASE("vertex_attributes_stride: sum of the attribute sizes") {
  std::vector<wgpu::VertexAttribute> attributes{{.format = Float32x2}, {.format = Float32x3}, {.format = Unorm8x4}};
  CHECK(lab::vertex_attributes_stride(attributes) == 8 + 12 + 4);
  CHECK(lab::vertex_attributes_stride({}) == 0);
}
