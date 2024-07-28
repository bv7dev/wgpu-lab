#ifndef WGPU_LAB_BUFFER_H
#define WGPU_LAB_BUFFER_H

#include "lab_webgpu.h"

#include <functional>
#include <vector>

namespace lab {

template<class T> //<class BufType, size_t NumElems>
struct ReadableBuffer {
  using WriteCallback = std::function<void(std::vector<T>&)>;
  using ReadCallback = std::function<void(const std::vector<T>&)>;

  // ReadableBuffer(Webgpu& inst, WriteCallback write_callback) : webgpu{inst} {
  //   wgpu::BufferDescriptor bufferDesc;
  //   bufferDesc.label = "My Buffer";
  //   bufferDesc.usage = wgpu::BufferUsage::MapRead |
  //   wgpu::BufferUsage::CopyDst; bufferDesc.size = sizeof(T) * NumElems;
  //   bufferDesc.mappedAtCreation = true;

  //   wgpu_buffer = webgpu.device.createBuffer(bufferDesc);

  //   void* map = wgpu_buffer.getMappedRange(0, sizeof(T) * data.size());

  //   std::vector<BufType> vmap{map, map + sizeof(T) * NumElems};

  //   write_callback(vmap);

  //   wgpu_buffer.unmap();
  // }

  ReadableBuffer(Webgpu& wg, const std::vector<T>& data) : webgpu{wg} {
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
