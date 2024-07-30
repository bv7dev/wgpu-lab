#ifndef WGPU_LAB_BUFFER_H
#define WGPU_LAB_BUFFER_H

#include "lab_webgpu.h"

#include <functional>
#include <ranges>
#include <thread>

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
      view = {};
      view_size = 0;
    }
  }

  ~MappedVRAM() { unmap(); }

  // public underlying container
  mutable std::span<T> view;

private:
  mutable wgpu::Buffer buffer;
  mutable size_t view_size;
};

template<typename T>
using ConstMappedVRAM = const MappedVRAM<const T>;

template<typename T>
struct ReadableBuffer {
  ReadableBuffer(const char* label, Webgpu& instance) : webgpu{instance}, label{label} {}

  ReadableBuffer(const char* label, const std::vector<T>& data, Webgpu& instance)
      : webgpu{instance}, label{label} {
    wgpu::BufferDescriptor bufferDesc{{
        .label = label,
        .usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst,
        .size = sizeof(T) * data.size(),
        .mappedAtCreation = true,
    }};
    current.capacity = data.size();
    wgpu_buffer = webgpu.device.createBuffer(bufferDesc);
    void* map = wgpu_buffer.getMappedRange(0, sizeof(T) * data.size());
    memcpy(map, data.data(), sizeof(T) * data.size());
    wgpu_buffer.unmap();
  }

  using WriteCallback = std::function<void(MappedVRAM<T>&&)>;
  void to_device(size_t capacity, WriteCallback write_func) {
    assert(wgpu_buffer == nullptr);
    wgpu::BufferDescriptor bufferDesc{{
        .label = label,
        .usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst,
        .size = sizeof(T) * capacity,
        .mappedAtCreation = true,
    }};
    wgpu_buffer = webgpu.device.createBuffer(bufferDesc);
    current.capacity = capacity;

    auto write_thread = std::thread(
        [](WriteCallback write_func, size_t capacity, wgpu::Buffer buffer) {
          auto map = reinterpret_cast<T*>(buffer.getMappedRange(0, sizeof(T) * capacity));
          MappedVRAM<T> vmap{{map, capacity}, 0, buffer};
          write_func(std::move(vmap));
        },
        write_func, capacity, wgpu_buffer);
    write_thread.detach();
  }

  using ReadCallback = std::function<void(ConstMappedVRAM<T>&&)>;
  auto from_device(size_t offset, size_t num_elems, ReadCallback read_func) {
    assert(wgpu_buffer != nullptr && current.mapping == false);
    current = {read_func, offset, num_elems, current.capacity, true};
    WGPUBufferMapCallbackInfo2 cbinfo = {
        .nextInChain = nullptr,
        .mode = wgpu::CallbackMode::AllowSpontaneous,
        .callback = map_callback,
        .userdata1 = this,
        .userdata2 = nullptr,
    };
    return wgpu_buffer.mapAsync2(wgpu::MapMode::Read, sizeof(T) * offset, sizeof(T) * num_elems,
                                 cbinfo);
  }

  auto from_device(ReadCallback read_func) { return from_device(0, current.capacity, read_func); }

  ~ReadableBuffer() {
    if (wgpu_buffer) {
      wgpu_buffer.release();
      wgpu_buffer = nullptr;
    }
  }

  wgpu::Buffer wgpu_buffer = nullptr;
  Webgpu& webgpu;

private:
  static void read_thread_func(ReadableBuffer* self) {
    ConstMappedVRAM<T> vmap{
        {reinterpret_cast<const T*>(self->wgpu_buffer.getConstMappedRange(
             sizeof(T) * self->current.offset, sizeof(T) * self->current.num_elems)),
         self->current.num_elems},
        self->current.num_elems,
        self->wgpu_buffer};
    self->current.user_callback(std::move(vmap));
    self->current = {nullptr, 0, 0, false, 0}; // user callback finished
  }

  static void map_callback(WGPUMapAsyncStatus status, char const*, void* userdata1, void*) {
    ReadableBuffer* self = reinterpret_cast<ReadableBuffer*>(userdata1);
    std::cout << "Info: Buffer: " << self->label << ": mapped with status: " << status << std::endl;
    if (status != WGPUMapAsyncStatus_Success) {
      std::cout << "Error: Buffer: mapping failed!" << std::endl;
      return;
    }
    auto read_thread = std::thread(read_thread_func, self);
    read_thread.detach();
  }

  struct AsyncContext {
    ReadCallback user_callback;
    size_t offset, num_elems;
    size_t capacity;
    bool mapping;
  } current{};

  const char* label;
};

} // namespace lab

#endif // WGPU_LAB_BUFFER_H
