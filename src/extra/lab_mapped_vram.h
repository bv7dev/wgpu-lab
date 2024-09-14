#ifndef WGPU_LAB_MAPPED_VRAM
#define WGPU_LAB_MAPPED_VRAM

#include <dawn/webgpu_cpp.h>

#include <ranges>

namespace lab {

// Convenience wrapper around `std::span` and `wgpu::Buffer`
// which unmaps buffer automatically on destruction.
// It's interface is similar to that of std::vector.
// Example:
// ```cpp
// auto write_func = [](MappedVRAM<T> vmap) {
//   for (int i = 0; i < vmap.capacity(); ++i) {
//     vmap.push(my_data[i]); // write data into vmap
//   }
// }
// const size_t capacity = 1024;
// buffer.to_device(capacity, write_func);
// ```
template<typename T>
struct MappedVRAM {
  MappedVRAM(std::span<T> view, size_t view_size, wgpu::Buffer wgpu_buffer)
      : data_view{view}, view_size{view_size}, wgpu_buffer{wgpu_buffer} {}
  MappedVRAM(MappedVRAM<T>&& other)
      : data_view{std::move(other.data_view)}, view_size{std::move(other.view_size)},
        wgpu_buffer{std::move(other.wgpu_buffer)} {
    other.data_view = {};
    other.view_size = 0;
    other.wgpu_buffer = nullptr;
  }
  MappedVRAM& operator=(MappedVRAM<T>&& other) {
    data_view = std::move(other.data_view);
    view_size = std::move(other.view_size);
    wgpu_buffer = std::move(other.wgpu_buffer);
    other.data_view = {};
    other.view_size = 0;
    other.wgpu_buffer = nullptr;
    return *this;
  }

  MappedVRAM(const MappedVRAM<T>&) = delete;
  MappedVRAM<T>& operator=(const MappedVRAM<T>&) = delete;

  std::span<T> get_view() const {
    assert(wgpu_buffer && view_size > 0);
    return {data_view.begin(), view_size};
  }

  size_t capacity() const { return data_view.size(); }
  size_t size() const { return view_size; }

  const T& operator[](size_t index) const {
    assert(wgpu_buffer && index < view_size);
    return data_view[index];
  }
  T& operator[](size_t index) {
    assert(wgpu_buffer && index < view_size);
    return data_view[index];
  }

  void resize(size_t size) {
    assert(wgpu_buffer && size <= capacity());
    view_size = size;
  }
  const T& push(const T& e) {
    assert(wgpu_buffer && view_size < capacity());
    return data_view[view_size++] = e;
  }
  T& pop() {
    assert(wgpu_buffer && view_size > 0);
    return data_view[--view_size];
  }

  auto begin() const { return data_view.begin(); }
  auto end() const { return data_view.begin() + view_size; }
  auto begin() { return data_view.begin(); }
  auto end() { return data_view.begin() + view_size; }

  bool is_mapped() { return wgpu_buffer != nullptr; }
  void unmap() {
    if (wgpu_buffer) {
      wgpu_buffer.Unmap();
      wgpu_buffer = nullptr;
      view_size = 0;
      data_view = {};
    }
  }
  ~MappedVRAM() { unmap(); }

  wgpu::Buffer wgpu_buffer;
  std::span<T> data_view;
  size_t view_size;
};

} // namespace lab

#endif // WGPU_LAB_MAPPED_VRAM
