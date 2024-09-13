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
    current_capacity = data.size();
    wgpu_buffer = webgpu.device.CreateBuffer(&bufferDesc);
    void* map = wgpu_buffer.GetMappedRange(0, sizeof(T) * data.size());
    memcpy(map, data.data(), sizeof(T) * data.size());
    wgpu_buffer.Unmap();
  }

  using WriteCallback = std::function<void(MappedVRAM<T>)>;
  std::jthread to_device(WriteCallback write_func, size_t capacity, wgpu::BufferUsage usage) {
    assert(wgpu_buffer == nullptr);
    wgpu::BufferDescriptor bufferDesc{
        .label = label,
        .usage = usage,
        .size = sizeof(T) * capacity,
        .mappedAtCreation = true,
    };
    wgpu_buffer = webgpu.device.CreateBuffer(&bufferDesc);
    current_capacity = capacity;
    return std::jthread{[](WriteCallback write_func, size_t capacity, wgpu::Buffer buffer) {
                          auto map = reinterpret_cast<T*>(buffer.GetMappedRange(0, sizeof(T) * capacity));
                          MappedVRAM<T> vmap{{map, capacity}, 0, buffer};
                          write_func(std::move(vmap));
                        },
                        write_func, capacity, wgpu_buffer};
  }

  using ReadCallback = std::function<void(MappedVRAM<const T>)>;
  std::jthread from_device(size_t offset, size_t num_elems, ReadCallback read_func) {
    assert(wgpu_buffer != nullptr);
    wgpu::Future future = wgpu_buffer.MapAsync(
        wgpu::MapMode::Read, sizeof(T) * offset, sizeof(T) * num_elems, wgpu::CallbackMode::WaitAnyOnly,
        [](wgpu::MapAsyncStatus status, char const* message, Buffer* self) {
          if (status == wgpu::MapAsyncStatus::Success) {
            std::cout << "Info: Buffer: " << self->label << " successfully mapped memory!" << std::endl;
          } else {
            std::cerr << "Error: Buffer: " << message << std::endl;
          }
        },
        this);
    webgpu.instance.WaitAny(future, UINT64_MAX);
    return std::jthread{[](ReadCallback read_func, size_t offset, size_t num_elems, wgpu::Buffer buffer) {
                          MappedVRAM<const T> vmap{{reinterpret_cast<const T*>(buffer.GetConstMappedRange(
                                                        sizeof(T) * offset, sizeof(T) * num_elems)),
                                                    num_elems},
                                                   num_elems,
                                                   buffer};
                          read_func(std::move(vmap));
                        },
                        read_func, offset, num_elems, wgpu_buffer};
  }
  inline std::jthread from_device(ReadCallback read_func) { return from_device(0, current_capacity, read_func); }

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
  size_t current_capacity;
  const char* label;
  Webgpu& webgpu;
};

} // namespace lab

#endif // WGPU_LAB_BUFFER_H
