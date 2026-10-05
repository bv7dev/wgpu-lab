#ifndef WGPU_LAB_BUFFER_H
#define WGPU_LAB_BUFFER_H

#include <lab_error.h>
#include <lab_gpu.h>
#include <lab_label.h>

#include <cstdint>
#include <format>
#include <functional>
#include <initializer_list>
#include <ranges>
#include <span>
#include <type_traits>
#include <vector>

namespace lab {

// Types whose bytes can be copied to and from GPU memory as they are
template<typename T>
concept GpuData = std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

namespace detail {

// The part of Buffer<T> that does not depend on T
struct BufferCore {
  using ReadCallback = std::function<void(const void* data, uint64_t byte_count)>;

  BufferCore(Gpu& gpu, uint64_t byte_size, wgpu::BufferUsage usage, Label label, const void* data,
             const std::function<void(void* mapped)>& fill);

  void write(uint64_t byte_offset, const void* data, uint64_t byte_count) const;
  void read(uint64_t byte_offset, void* out, uint64_t byte_count) const;
  void read_async(uint64_t byte_offset, uint64_t byte_count, ReadCallback callback) const;

  std::shared_ptr<GpuState> gpu;
  wgpu::Buffer handle;
  wgpu::BufferUsage usage;
  uint64_t byte_size = 0;
  std::string label;
};

} // namespace detail

// A typed array in GPU memory: vertices, indices, uniforms, storage, ...
// ```cpp
// lab::Buffer vertices(gpu, vertex_data);                               // Buffer<MyVertex>, a vertex buffer
// lab::Buffer<MyUniforms> uniforms(gpu, {my_uniforms}, wgpu::BufferUsage::Uniform);
// uniforms.write(my_uniforms);
// std::vector<float> result = storage.read();
// ```
// The lab adds the usage flags that `write()` and `read()` need, so only the
// purpose of the buffer has to be given.
template<GpuData T>
class Buffer {
public:
  static constexpr size_t all = ~size_t{0};

  // Creates a buffer that holds a copy of `data`
  Buffer(Gpu& gpu, std::span<const T> data, wgpu::BufferUsage usage = wgpu::BufferUsage::Vertex, Label label = {})
      : core{gpu, data.size_bytes(), usage, std::move(label).as("buffer"), data.data(), nullptr},
        element_count{data.size()} {}

  Buffer(Gpu& gpu, std::initializer_list<T> data, wgpu::BufferUsage usage = wgpu::BufferUsage::Vertex, Label label = {})
      : Buffer{gpu, std::span<const T>{data.begin(), data.size()}, usage, std::move(label)} {}

  // Creates a buffer of `count` elements with all bytes set to zero
  Buffer(Gpu& gpu, size_t count, wgpu::BufferUsage usage, Label label = {})
      : core{gpu, count * sizeof(T), usage, std::move(label).as("buffer"), nullptr, nullptr}, element_count{count} {}

  // Creates a buffer of `count` elements and lets `fill` write them straight into GPU memory
  // ```cpp
  // lab::Buffer<float> ramp(gpu, 256, wgpu::BufferUsage::Storage,
  //                         [](std::span<float> mapped) { std::ranges::iota(mapped, 0.0f); });
  // ```
  Buffer(Gpu& gpu, size_t count, wgpu::BufferUsage usage, const std::function<void(std::span<T>)>& fill,
         Label label = {})
      : core{gpu,     count * sizeof(T),
             usage,   std::move(label).as("buffer"),
             nullptr, [&](void* mapped) { fill(std::span<T>{static_cast<T*>(mapped), count}); }},
        element_count{count} {}

  Buffer(Buffer&&) = default;
  Buffer& operator=(Buffer&&) = default;

  // Replaces elements of the buffer, starting at index `first`
  void write(std::span<const T> data, size_t first = 0) {
    check_range(first, data.size(), "write");
    core.write(first * sizeof(T), data.data(), data.size_bytes());
  }

  // Replaces the element at `index`
  void write(const T& element, size_t index = 0) { write(std::span<const T>{&element, 1}, index); }

  // Copies `count` elements starting at `first` back from the GPU
  //  - blocks until the GPU has caught up, use `read_async()` to keep rendering meanwhile
  std::vector<T> read(size_t first = 0, size_t count = all) const {
    count = count == all ? element_count - std::min(first, element_count) : count;
    check_range(first, count, "read");
    std::vector<T> result(count);
    core.read(first * sizeof(T), result.data(), count * sizeof(T));
    return result;
  }

  // Like `read()`, but returns immediately: `callback` is called with the elements
  // from within a later `tick()` (or `Gpu::poll()`)
  void read_async(std::function<void(std::span<const T>)> callback, size_t first = 0, size_t count = all) const {
    count = count == all ? element_count - std::min(first, element_count) : count;
    check_range(first, count, "read_async");
    core.read_async(first * sizeof(T), count * sizeof(T),
                    [callback = std::move(callback)](const void* data, uint64_t byte_count) {
                      callback(std::span<const T>{static_cast<const T*>(data), byte_count / sizeof(T)});
                    });
  }

  // number of elements
  size_t size() const { return element_count; }

  const wgpu::Buffer& handle() const { return core.handle; }
  const std::string& label() const { return core.label; }

private:
  void check_range(size_t first, size_t count, const char* what) const {
    if (first > element_count || count > element_count - first) {
      detail::fail(core.label, std::format("{}: elements [{}, {}) are outside of the buffer, which has {} elements",
                                           what, first, first + count, element_count));
    }
  }

  detail::BufferCore core;
  size_t element_count = 0;
};

// lets `lab::Buffer buffer(gpu, my_vector)` find out the element type
template<std::ranges::contiguous_range R>
Buffer(Gpu&, R&&, wgpu::BufferUsage = wgpu::BufferUsage::Vertex, Label = {})
    -> Buffer<std::remove_cv_t<std::ranges::range_value_t<R>>>;

} // namespace lab

#endif // WGPU_LAB_BUFFER_H
