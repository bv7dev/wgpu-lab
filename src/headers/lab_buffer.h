#ifndef WGPU_LAB_BUFFER_H
#define WGPU_LAB_BUFFER_H

#include "lab_webgpu.h"

#include <array>
#include <functional>
#include <vector>

namespace lab {

template<typename T>
struct MappedVRAM {
  MappedVRAM(T* data, size_t capacity, wgpu::Buffer buffer)
      : _data{data}, _capacity{capacity}, _buffer{buffer}, _size{0} {}
  T* begin() { return _data; }
  T* end() { return _data + _capacity; }
  const T* begin() const { return _data; }
  const T* end() const { return _data + _capacity; }
  size_t size() const { return _size; }
  size_t capacity() const { return _capacity; }
  T operator[](size_t i) const {
    assert(_data != nullptr && _capacity != 0 && i < _capacity);
    return _data[i];
  }
  T& operator[](size_t i) {
    assert(_data != nullptr && _capacity != 0 && i < _capacity);
    return _data[i];
  }
  void push(const T& e) { _data[_size++] = e; }
  T pop() { return _data[--_size]; }
  void unmap() { ~MappedVRAM(); }
  ~MappedVRAM() {
    std::cout << "~unmapping vmap...\n";
    _buffer.unmap();
  }

private:
  T* _data;
  size_t _capacity;
  size_t _size;
  wgpu::Buffer _buffer;
};

template<typename T>
struct ReadableBuffer {

  using WriteCallback = std::function<void(MappedVRAM<T>&)>;
  using ReadCallback = std::function<void(const std::vector<T>&)>;

  ReadableBuffer(Webgpu& instance, size_t capacity, WriteCallback callback)
      : webgpu{instance} {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.label = "My Readable Buffer";
    bufferDesc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    bufferDesc.size = sizeof(T) * capacity;
    bufferDesc.mappedAtCreation = true;

    wgpu_buffer = webgpu.device.createBuffer(bufferDesc);

    MappedVRAM<T> vmap{reinterpret_cast<T*>(
                           wgpu_buffer.getMappedRange(0, sizeof(T) * capacity)),
                       capacity, wgpu_buffer};

    callback(vmap);
  }

  // ReadableBuffer(Webgpu& wg, const std::vector<T>& data) : webgpu{wg} {
  //   wgpu::BufferDescriptor bufferDesc;
  //   bufferDesc.label = "My Buffer";
  //   bufferDesc.usage = wgpu::BufferUsage::MapRead |
  //   wgpu::BufferUsage::CopyDst; bufferDesc.size = sizeof(T) * data.size();
  //   bufferDesc.mappedAtCreation = true;

  //   wgpu_buffer = webgpu.device.createBuffer(bufferDesc);

  //   void* map = wgpu_buffer.getMappedRange(0, sizeof(T) * data.size());
  //   memcpy(map, data.data(), sizeof(T) * data.size());
  //   wgpu_buffer.unmap();
  // }

  struct MapBufferFuture {
    std::vector<T> data{};
    bool is_ready() { return size != 0 && data.size() != 0; }
    size_t offset = 0, size = 0;
  } _internal_map;

  const MapBufferFuture& read_async(size_t offset, size_t size,
                                    ReadCallback user_callback) {
    std::cout << "read_async call..." << std::endl;

    _internal_map.offset = offset;
    _internal_map.size = size;

    WGPUBufferMapCallbackInfo2 cbinfo{};
    cbinfo.mode = wgpu::CallbackMode::AllowSpontaneous;
    cbinfo.userdata1 = this;
    cbinfo.userdata2 = &user_callback;
    cbinfo.callback = [](WGPUMapAsyncStatus status, char const* msg,
                         void* userdata1, void* userdata2) {
      std::cout << "Buffer mapped with status: " << status << std::endl;
      if (status != WGPUMapAsyncStatus_Success) {
        std::cout << "Error: read_async: mapping failed!" << std::endl;
        return;
      }

      ReadableBuffer* self = reinterpret_cast<ReadableBuffer*>(userdata1);

      const void* buf = self->wgpu_buffer.getConstMappedRange(
          sizeof(T) * self->_internal_map.offset,
          sizeof(T) * self->_internal_map.size);

      self->_internal_map.data.resize(self->_internal_map.size);
      memcpy(self->_internal_map.data.data(), buf,
             sizeof(T) * self->_internal_map.size);

      (*reinterpret_cast<ReadCallback*>(userdata2))(self->_internal_map.data);

      self->wgpu_buffer.unmap();

      // if (*userdata2) {
      //   std::vector<T> test{1, 2, 3, 4};
      //   (*reinterpret_cast<ReadCallback*>(userdata2))(test);
      // }

      // MapBufferFuture* imap =
      // reinterpret_cast<MapBufferFuture*>(userdata1);

      // memcpy(imap->data, vmap, sizeof(T) * 3);
      // imap->is_ready = true;

      // Todo: I don't know yet what this msg parameter is
      if (msg) std::cout << "read_async: message: " << msg << std::endl;
    };

    wgpu_buffer.mapAsync2(wgpu::MapMode::Read, sizeof(T) * offset,
                          sizeof(T) * size, cbinfo);

    // wgpuBufferMapAsync2(wgpu_buffer, WGPUMapMode_Read, sizeof(T) * offset,
    //                     sizeof(T) * size, cbinfo);

    return _internal_map;
  }

  void unmap() {
    read_map = nullptr;
    wgpu_buffer.unmap();
  }

  ~ReadableBuffer() {
    if (wgpu_buffer) {
      wgpu_buffer.release();
      wgpu_buffer = nullptr;
    }
  }

  wgpu::Buffer wgpu_buffer;
  Webgpu& webgpu;

  const void* read_map = nullptr;
};

} // namespace lab

#endif // WGPU_LAB_BUFFER_H
