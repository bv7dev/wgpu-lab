#ifndef WGPU_LAB_BUFFER_H
#define WGPU_LAB_BUFFER_H

#include "lab_webgpu.h"

#include <webgpu/webgpu.hpp>

#include <vector>

namespace lab {

struct ReadMap {
  const void* data = nullptr;
};

struct WriteMap {
  void* data = nullptr;
};

struct Buffer {
  Buffer(size_t bytes, Webgpu& wg) : webgpu{wg}, size{bytes} {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.label = "My Buffer";
    bufferDesc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    bufferDesc.size = bytes;
    bufferDesc.mappedAtCreation = false;
    wgpu_buffer = webgpu.device.createBuffer(bufferDesc);
    // write_map.data = wgpu_buffer.getMappedRange(0, size);
  }

  void write(uint64_t offset, size_t length, const void* data) {
    assert(length <= size);
    webgpu.queue.writeBuffer(wgpu_buffer, offset, data, length);
  }

  WriteMap& get_write_map() {
    if (!write_map.data) {
      cbfunc = wgpu_buffer.mapAsync(
          wgpu::MapMode::Write, 0, size,
          [&](wgpu::BufferMapAsyncStatus status) {
            std::cout << "buffer mapped with status: " << status << std::endl;
            write_map.data = wgpu_buffer.getMappedRange(0, size);
          });
    }
    return write_map;
  }

  const ReadMap& get_read_map() {
    if (!read_map.data) {
      cbfunc = wgpu_buffer.mapAsync(
          wgpu::MapMode::Read, 0, size, [&](wgpu::BufferMapAsyncStatus status) {
            std::cout << "buffer mapped with status: " << status << std::endl;
            read_map.data = wgpu_buffer.getConstMappedRange(0, size);
          });
    }
    return read_map;
  }

  void unmap() {
    write_map.data = nullptr;
    read_map.data = nullptr;
    wgpu_buffer.unmap();
  }

  ~Buffer() {
    if (wgpu_buffer) {
      wgpu_buffer.release();
      wgpu_buffer = nullptr;
    }
  }
  std::unique_ptr<wgpu::BufferMapCallback> cbfunc;
  wgpu::Buffer wgpu_buffer;
  const size_t size;
  Webgpu& webgpu;

  WriteMap write_map;
  ReadMap read_map;
};

} // namespace lab

#endif // WGPU_LAB_BUFFER_H
