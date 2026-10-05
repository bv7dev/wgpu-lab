#include <doctest/doctest.h>

#include <lab>

TEST_CASE("Buffer: written elements can be read back") {
  lab::Webgpu webgpu("test");
  REQUIRE(webgpu.device);

  std::vector<int> data{1, 2, 3, 4, 5, 6, 7, 8};
  lab::Buffer<int> buffer("test buffer", data, wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst, webgpu);

  buffer.write(42, 2);                     // one element at index 2
  buffer.write(std::vector<int>{7, 7}, 6); // two elements starting at index 6

  std::vector<int> result;
  {
    auto reader = buffer.from_device(
        [&result](lab::MappedVRAM<const int> mapped) { result.assign(mapped.begin(), mapped.end()); });
  } // the reader thread is joined here

  CHECK(result == std::vector<int>{1, 2, 42, 4, 5, 6, 7, 7});
}

TEST_CASE("Buffer: a part of the buffer can be read back") {
  lab::Webgpu webgpu("test");
  REQUIRE(webgpu.device);

  std::vector<float> data{0.5f, 1.5f, 2.5f, 3.5f, 4.5f};
  lab::Buffer<float> buffer("test buffer", data, wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst, webgpu);

  std::vector<float> result;
  {
    // offsets are multiples of 8 bytes, the alignment WebGPU requires for mapping
    auto reader = buffer.from_device(
        2, 2, [&result](lab::MappedVRAM<const float> mapped) { result.assign(mapped.begin(), mapped.end()); });
  }

  CHECK(result == std::vector<float>{2.5f, 3.5f});
}
