#ifndef WGPU_LAB_BUFFER_H
#define WGPU_LAB_BUFFER_H

#include <dawn/webgpu_cpp.h>

#include <extra/lab_mapped_vram.h>
#include <objects/lab_webgpu.h>

#include <functional>
#include <iostream>
#include <thread>

namespace lab {

template<typename T>
struct Buffer {
  Buffer(const char* label, Webgpu& instance) : webgpu{instance}, label{label} {}

  Buffer(const char* label, const std::vector<T>& data, Webgpu& instance) : Buffer{label, instance} {
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
          auto map = reinterpret_cast<T*>(buffer.GetMappedRange(0, sizeof(T) * capacity));
          MappedVRAM<T> vmap{{map, capacity}, 0, buffer};
          write_func(std::move(vmap));
        },
        write_func, capacity, wgpu_buffer);
    write_thread.detach();
  }

  using ReadCallback = std::function<void(ConstMappedVRAM<T>&&)>;
  void from_device(size_t offset, size_t num_elems, ReadCallback read_func) {
    assert(wgpu_buffer != nullptr && current.mapping == false);
    current = {read_func, offset, num_elems, current.capacity, true};
    wgpu::Future future = wgpu_buffer.MapAsync(
        wgpu::MapMode::Read, sizeof(T) * offset, sizeof(T) * num_elems, wgpu::CallbackMode::WaitAnyOnly,
        [](wgpu::MapAsyncStatus status, char const* message, Buffer* self) {
          if (status == wgpu::MapAsyncStatus::Success) {
            std::cout << "Info: Buffer: " << self->label << " successfully mapped memory!" << std::endl;
            auto read_thread = std::thread(
                [](Buffer* self) {
                  ConstMappedVRAM<T> vmap{{reinterpret_cast<const T*>(self->wgpu_buffer.GetConstMappedRange(
                                               sizeof(T) * self->current.offset, sizeof(T) * self->current.num_elems)),
                                           self->current.num_elems},
                                          self->current.num_elems,
                                          self->wgpu_buffer};
                  self->current.user_callback(std::move(vmap));
                  self->current = {nullptr, 0, 0, false, 0}; // user callback finished
                },
                self);
            read_thread.detach();
          } else {
            std::cerr << "Error: Buffer: " << message << std::endl;
          }
        },
        this);
    webgpu.instance.WaitAny(future, UINT64_MAX);
  }
  auto from_device(ReadCallback read_func) { return from_device(0, current.capacity, read_func); }

  // write a single element to an existing buffer (needs CopyDst flag)
  void write(const T& elem, uint64_t offset = 0) {
    webgpu.queue.WriteBuffer(wgpu_buffer, offset * sizeof(T), &elem, sizeof(T));
  }

  // write a vector of elements to an existing buffer (needs CopyDst flag)
  void write(const std::vector<T>& data, uint64_t offset = 0) {
    webgpu.queue.WriteBuffer(wgpu_buffer, offset * sizeof(T), data.data(), data.size() * sizeof(T));
  }

  ~Buffer() {
    if (wgpu_buffer) {
      wgpu_buffer.Destroy();
      wgpu_buffer = nullptr;
    }
  }

  wgpu::Buffer wgpu_buffer = nullptr;
  Webgpu& webgpu;

private:
  struct ReadThreadContext {
    ReadCallback user_callback;
    size_t offset, num_elems;
    size_t capacity;
    bool mapping;
  } current{};

  const char* label;
};

} // namespace lab

#endif // WGPU_LAB_BUFFER_H
