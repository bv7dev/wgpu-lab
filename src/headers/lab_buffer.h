#ifndef WGPU_LAB_BUFFER_H
#define WGPU_LAB_BUFFER_H

#include "lab_webgpu.h"

#include <functional>

namespace lab {

template<typename T>
class MappedVRAM {
  wgpu::Buffer _buffer;
  size_t _capacity;
  size_t _size;
  T* _data;

public:
  MappedVRAM(T* data, size_t capacity, size_t size, wgpu::Buffer buffer)
      : _data{data}, _capacity{capacity}, _buffer{buffer}, _size{size} {}

  const T* begin() const { return _data; }
  const T* end() const { return _data + _size; }
  T* begin() { return _data; }
  T* end() { return _data + _size; }

  size_t capacity() const { return _capacity; }
  size_t size() const { return _size; }

  T operator[](size_t i) const {
    assert(_data != nullptr && _capacity != 0 && i < _size);
    return _data[i];
  }
  T& operator[](size_t i) {
    assert(_data != nullptr && _capacity != 0 && i < _size);
    return _data[i];
  }

  void push(const T& e) { _data[_size++] = e; }
  T pop() { return _data[--_size]; }

  ~MappedVRAM() {
    std::cout << "~unmap\n";
    _buffer.unmap();
  }
  void unmap() { ~MappedVRAM(); }
};

template<typename T>
struct ReadableBuffer {
  using WriteCallback = std::function<void(MappedVRAM<T>&)>;
  using ReadCallback = std::function<void(const MappedVRAM<const T>&)>;

  const char* _label;

  ReadableBuffer(const char* label, Webgpu& instance)
      : webgpu{instance}, _label{label} {}

  void to_device(size_t capacity, WriteCallback callback) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.label = _label;
    bufferDesc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    bufferDesc.size = sizeof(T) * capacity;
    bufferDesc.mappedAtCreation = true;

    wgpu_buffer = webgpu.device.createBuffer(bufferDesc);

    MappedVRAM<T> vmap{reinterpret_cast<T*>(
                           wgpu_buffer.getMappedRange(0, sizeof(T) * capacity)),
                       capacity, 0, wgpu_buffer};
    callback(vmap);
  }

  ReadableBuffer(Webgpu& webgpu, const std::vector<T>& data) : webgpu{webgpu} {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.label = "My Buffer";
    bufferDesc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    bufferDesc.size = sizeof(T) * data.size();
    bufferDesc.mappedAtCreation = true;

    wgpu_buffer = webgpu.device.createBuffer(bufferDesc);

    void* map = wgpu_buffer.getMappedRange(0, sizeof(T) * data.size());
    memcpy(map, data.data(), sizeof(T) * data.size());
    wgpu_buffer.unmap();
  }

  auto read_async(size_t offset, size_t num_elems, ReadCallback callback) {
    assert(wgpu_buffer != nullptr && mapping_active == false);

    mapping_active = true;
    current_callback = callback;
    current_offset = offset;
    current_num_elems = num_elems;

    WGPUBufferMapCallbackInfo2 cbinfo{};
    cbinfo.mode = wgpu::CallbackMode::AllowSpontaneous;
    cbinfo.userdata1 = this;
    cbinfo.userdata2 = nullptr;
    cbinfo.callback = [](WGPUMapAsyncStatus status, char const* msg,
                         void* userdata1, void*) {
      std::cout << "Buffer mapped with status: " << status << std::endl;
      if (status != WGPUMapAsyncStatus_Success) {
        std::cout << "Error: read_async: mapping failed!" << std::endl;
        return;
      }

      ReadableBuffer* self = reinterpret_cast<ReadableBuffer*>(userdata1);

      MappedVRAM<const T> vmap(
          reinterpret_cast<const T*>(self->wgpu_buffer.getConstMappedRange(
              sizeof(T) * self->current_offset,
              sizeof(T) * self->current_num_elems)),
          self->current_num_elems, self->current_num_elems, self->wgpu_buffer);

      self->current_callback(vmap);

      self->current_callback = nullptr;
      self->current_num_elems = 0;
      self->current_offset = 0;
      self->mapping_active = false;

      // TODO: I don't know yet what this msg parameter is
      if (msg) std::cout << "read_async: message: " << msg << std::endl;
    };

    return wgpu_buffer.mapAsync2(wgpu::MapMode::Read, sizeof(T) * offset,
                                 sizeof(T) * num_elems, cbinfo);

    // // Deprecated mapAsync --------------------------------------------------
    // return wgpu_buffer.mapAsync(
    //     wgpu::MapMode::Read, sizeof(T) * offset, sizeof(T) * num_elems,
    //     [&](wgpu::BufferMapAsyncStatus status) {
    //       if (status != wgpu::BufferMapAsyncStatus::Success) {
    //         std::cout << "Error: MapAsync status: " << status << std::endl;
    //         return;
    //       }
    //       MappedVRAM<const T> vmap(
    //           reinterpret_cast<const T*>(wgpu_buffer.getConstMappedRange(
    //               sizeof(T) * current_offset, sizeof(T) *
    //               current_num_elems)),
    //           current_num_elems, current_num_elems, wgpu_buffer);
    //       current_callback(vmap);
    //       current_callback = nullptr;
    //       mapping_active = false;
    //       current_num_elems = 0;
    //       current_offset = 0;
    //     });
  }

  ~ReadableBuffer() {
    if (wgpu_buffer) {
      wgpu_buffer.release();
      wgpu_buffer = nullptr;
    }
  }

  wgpu::Buffer wgpu_buffer = nullptr;
  Webgpu& webgpu;

private:
  size_t current_offset, current_num_elems;
  ReadCallback current_callback;
  bool mapping_active = false;
};

} // namespace lab

#endif // WGPU_LAB_BUFFER_H
