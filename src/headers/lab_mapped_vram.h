#ifndef WGPU_LAB_MAPPED_VRAM
#define WGPU_LAB_MAPPED_VRAM

#include <webgpu/webgpu.hpp>

#include <ranges>

namespace lab {

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

  size_t capacity() const { return view.size(); }
  size_t size() const { return view_size; }

  void resize(size_t size) {
    assert(size <= capacity());
    view_size = size;
  }

  const T& push(const T& e) { return view[view_size++] = e; }
  T& pop() { return view[--view_size]; }

  auto begin() const { return view.begin(); }
  auto end() const { return view.begin() + view_size; }
  auto begin() { return view.begin(); }
  auto end() { return view.begin() + view_size; }

  void unmap() const {
    if (buffer) {
      buffer.unmap();
      buffer = nullptr;
      view_size = 0;
      view = {};
    }
  }
  ~MappedVRAM() { unmap(); }

  // public underlying container
  // can be used in stl algorithms
  mutable std::span<T> view;

private:
  mutable wgpu::Buffer buffer;
  mutable size_t view_size;
};

template<typename T>
using ConstMappedVRAM = const MappedVRAM<const T>;

} // namespace lab

#endif // WGPU_LAB_MAPPED_VRAM
