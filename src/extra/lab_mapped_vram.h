#ifndef WGPU_LAB_MAPPED_VRAM
#define WGPU_LAB_MAPPED_VRAM

#include <webgpu/webgpu.hpp>

#include <ranges>

namespace lab {

// Convenience wrapper around `std::span` and `wgpu::Buffer`
// which unmaps buffer automatically on destruction.
// It's interface mimics that of a simple vector.
// Example:
// ```cpp
// auto write_func = [](MappedVRAM&& vmap) {
//   for (int i = 0; i < vmap.capacity(); ++i) {
//     vmap.push(my_data[i]); // write data into vmap
//   }
// }
// const size_t capacity = 1024;
// buffer.to_device(capacity, write_func);
// ```
template<typename T>
struct MappedVRAM {
  MappedVRAM(std::span<T> view, size_t size, wgpu::Buffer buffer)
      : view{view}, view_size{size}, buffer{buffer} {}
  MappedVRAM(MappedVRAM&& rhs) : view{rhs.view}, view_size{rhs.view_size}, buffer{rhs.buffer} {
    rhs.buffer = nullptr;
  }
  MappedVRAM& operator=(MappedVRAM&& rhs) {
    view = std::move(rhs.view);
    view_size = std::move(rhs.view_size);
    buffer = std::move(rhs.buffer);
    rhs.buffer = nullptr;
    return *this;
  }

  MappedVRAM(const MappedVRAM&) = delete;
  MappedVRAM& operator=(const MappedVRAM&) = delete;

  std::span<T> get_view() {
    assert(buffer && view_size > 0);
    return {view.begin(), view_size};
  }

  size_t capacity() const { return view.size(); }
  size_t size() const { return view_size; }

  const T& operator[](size_t index) const {
    assert(buffer && index < view_size);
    return view[index];
  }
  T& operator[](size_t index) {
    assert(buffer && index < view_size);
    return view[index];
  }

  void resize(size_t size) {
    assert(buffer && size <= capacity());
    view_size = size;
  }
  const T& push(const T& e) {
    assert(buffer && view_size < capacity());
    return view[view_size++] = e;
  }
  T& pop() {
    assert(buffer && view_size > 0);
    return view[--view_size];
  }

  auto begin() const { return view.begin(); }
  auto end() const { return view.begin() + view_size; }
  auto begin() { return view.begin(); }
  auto end() { return view.begin() + view_size; }

  bool is_mapped() { return buffer != nullptr; }
  void unmap() const {
    if (buffer) {
      buffer.unmap();
      buffer = nullptr;
      view_size = 0;
      view = {};
    }
  }
  ~MappedVRAM() { unmap(); }

private:
  // these are mutable so that unmap can clear them even in const context
  mutable wgpu::Buffer buffer;
  mutable std::span<T> view;
  mutable size_t view_size;
};

// Constant version of `MappedVRAM<T>`
// Example:
// ```cpp
// auto write_func = [](ConstMappedVRAM&& vmap) {
//   for (auto& e : vmap) {
//     std::cout << e << ' '; // read data from vmap
//   }
// }
// const size_t offset = 0, num_elems = 1024;
// buffer.from_device(offset, num_elems, read_func);
// ```
template<typename T>
using ConstMappedVRAM = const MappedVRAM<const T>;

} // namespace lab

#endif // WGPU_LAB_MAPPED_VRAM
