#ifndef WGPU_LAB_BUFFER_H
#define WGPU_LAB_BUFFER_H

#include <extra/lab_mapped_vram.h>
#include <objects/lab_webgpu.h>

#include <functional>
#include <iostream>
#include <thread>

namespace lab {

template<typename T>
struct Buffer {
  Buffer(const char* label, Webgpu& instance) : webgpu{instance}, label{label} {}

  Buffer(const char* label, const std::vector<T>& data, Webgpu& instance)
      : Buffer{label, instance} {
    to_device(data, wgpu::BufferUsage::Vertex);
  }

  Buffer(const char* label, const std::vector<T>& data, wgpu::BufferUsage usage, Webgpu& instance)
      : Buffer{label, instance} {
    to_device(data, usage);
  }

  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  void to_device(const std::vector<T>& data, wgpu::BufferUsage usage) {
    assert(wgpu_buffer == nullptr);
    wgpu::BufferDescriptor bufferDesc{
        .label = label,
        .usage = usage,
        .size = sizeof(T) * data.size(),
        .mappedAtCreation = true,
    };
    current.capacity = data.size();
    wgpu_buffer = webgpu.device.CreateBuffer(&bufferDesc);
    void* map = wgpu_buffer.GetMappedRange(0, sizeof(T) * data.size());
    memcpy(map, data.data(), sizeof(T) * data.size());
    wgpu_buffer.Unmap();
  }

  using WriteCallback = std::function<void(MappedVRAM<T>&&)>;
  void to_device(WriteCallback write_func, size_t capacity, wgpu::BufferUsage usage) {
    assert(wgpu_buffer == nullptr);
    wgpu::BufferDescriptor bufferDesc{
        .label = label,
        .usage = usage,
        .size = sizeof(T) * capacity,
        .mappedAtCreation = true,
    };
    wgpu_buffer = webgpu.device.CreateBuffer(&bufferDesc);
    current.capacity = capacity;

    auto write_thread = std::thread(
        [](WriteCallback write_func, size_t capacity, wgpu::Buffer buffer) {
          // auto map = reinterpret_cast<T*>(buffer.getMappedRange(0, sizeof(T) * capacity));
          auto map = new T[capacity];
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
    // WGPUBufferMapCallbackInfo2 cbinfo = {
    //     .nextInChain = nullptr,
    //     .mode = wgpu::CallbackMode::AllowSpontaneous,
    //     .callback = map_callback,
    //     .userdata1 = this,
    //     .userdata2 = nullptr,
    // };
    // return wgpu_buffer.mapAsync2(wgpu::MapMode::Read, sizeof(T) * offset, sizeof(T) * num_elems,
    //                              cbinfo);
  }
  auto from_device(ReadCallback read_func) { return from_device(0, current.capacity, read_func); }

  ~Buffer() {
    if (wgpu_buffer) {
      wgpu_buffer.Destroy();
      wgpu_buffer = nullptr;
    }
  }

  wgpu::Buffer wgpu_buffer = nullptr;
  Webgpu& webgpu;

private:
  static void read_thread_func(Buffer* self) {
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
    Buffer* self = reinterpret_cast<Buffer*>(userdata1);
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

  // WIP uniforms

public:
  // write a single element to an existing buffer (needs CopyDst flag)
  void write(const T& elem, uint64_t offset = 0) {
    webgpu.queue.WriteBuffer(wgpu_buffer, offset * sizeof(T), &elem, sizeof(T));
  }

  // write a vector of elements to an existing buffer (needs CopyDst flag)
  void write(const std::vector<T>& data, uint64_t offset = 0) {
    webgpu.queue.WriteBuffer(wgpu_buffer, offset * sizeof(T), data.data(), data.size() * sizeof(T));
  }
};

} // namespace lab

#endif // WGPU_LAB_BUFFER_H
