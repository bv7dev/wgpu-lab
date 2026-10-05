#include "common.h"

#include <numeric>

TEST_CASE("Buffer: the element type is deduced from the data") {
  lab::Gpu gpu;
  std::vector<float> data{1.0f, 2.0f, 3.0f};

  lab::Buffer buffer(gpu, data);

  static_assert(std::is_same_v<decltype(buffer), lab::Buffer<float>>);
  CHECK(buffer.size() == 3);
  CHECK(buffer.read() == data);
}

TEST_CASE("Buffer: written elements can be read back") {
  lab::Gpu gpu;
  lab::Buffer<int> buffer(gpu, {1, 2, 3, 4, 5, 6, 7, 8}, wgpu::BufferUsage::Storage);

  buffer.write(42, 2);                     // one element at index 2
  buffer.write(std::vector<int>{7, 7}, 6); // two elements starting at index 6

  CHECK(buffer.read() == std::vector<int>{1, 2, 42, 4, 5, 6, 7, 7});
  CHECK(buffer.read(1, 3) == std::vector<int>{2, 42, 4}); // a part that starts at an odd byte offset
  CHECK(buffer.read(8).empty());
}

TEST_CASE("Buffer: a buffer that can be mapped is read without a copy") {
  lab::Gpu gpu;
  lab::Buffer<int> buffer(gpu, {1, 2, 3, 4, 5}, wgpu::BufferUsage::MapRead);

  CHECK(buffer.read(3, 2) == std::vector<int>{4, 5});
}

TEST_CASE("Buffer: created with a count, it is zeroed or filled in place") {
  lab::Gpu gpu;

  lab::Buffer<uint32_t> zeroed(gpu, 4, wgpu::BufferUsage::Storage);
  CHECK(zeroed.read() == std::vector<uint32_t>{0, 0, 0, 0});

  lab::Buffer<float> filled(gpu, 5, wgpu::BufferUsage::Storage,
                            [](std::span<float> mapped) { std::iota(mapped.begin(), mapped.end(), 10.0f); });
  CHECK(filled.read() == std::vector<float>{10.0f, 11.0f, 12.0f, 13.0f, 14.0f});
}

TEST_CASE("Buffer: element sizes that are not a multiple of 4 bytes") {
  lab::Gpu gpu;
  lab::Buffer<uint16_t> indices(gpu, {0, 1, 2, 2, 3}, wgpu::BufferUsage::Index); // 10 bytes

  CHECK(indices.read() == std::vector<uint16_t>{0, 1, 2, 2, 3});

  indices.write(std::vector<uint16_t>{9, 9}, 2); // 4 bytes at byte 4: fine
  CHECK(indices.read() == std::vector<uint16_t>{0, 1, 9, 9, 3});

  indices.write(uint16_t{7}, 4); // 2 bytes, but up to the end of the buffer: fine
  CHECK(indices.read() == std::vector<uint16_t>{0, 1, 9, 9, 7});

  // 2 bytes in the middle cannot be written
  CHECK_THROWS_WITH_AS(indices.write(uint16_t{5}, 0), doctest::Contains("multiples of 4 bytes"), lab::Error);
}

TEST_CASE("Buffer: reading and writing outside of the buffer is an error") {
  lab::Gpu gpu;
  lab::Buffer<int> buffer(gpu, {1, 2, 3}, wgpu::BufferUsage::Storage, "three ints");

  CHECK_THROWS_WITH_AS(buffer.write(5, 3), doctest::Contains("three ints"), lab::Error);
  CHECK_THROWS_WITH_AS(buffer.read(2, 2), doctest::Contains("has 3 elements"), lab::Error);
}

TEST_CASE("Buffer: read_async delivers the elements from a later poll") {
  lab::Gpu gpu;
  lab::Buffer<int> buffer(gpu, {1, 2, 3, 4}, wgpu::BufferUsage::Storage);

  std::vector<int> result;
  buffer.read_async([&](std::span<const int> elements) { result.assign(elements.begin(), elements.end()); }, 1, 2);
  CHECK(result.empty()); // not yet

  gpu.wait_idle();
  gpu.poll();
  CHECK(result == std::vector<int>{2, 3});
}

TEST_CASE("Buffer: an unnamed buffer is labelled with the place it was created at") {
  lab::Gpu gpu;
  lab::Buffer<int> unnamed(gpu, {1});
  lab::Buffer<int> named(gpu, {1}, wgpu::BufferUsage::Vertex, "my buffer");

  CHECK(unnamed.label() == std::format("buffer test_buffer.cpp:{}", __LINE__ - 3));
  CHECK(named.label() == "my buffer");
}
