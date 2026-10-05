#include "lab_detail.h"

#include <lab_buffer.h>

#include <algorithm>
#include <cstring>

namespace lab::detail {

namespace {

// WebGPU wants buffer sizes, copy ranges and mapped ranges in multiples of 4 bytes
constexpr uint64_t align_up_4(uint64_t value) { return (value + 3) & ~uint64_t{3}; }

// The usage flags that make write() and read() work, on top of what the buffer is for.
// A mappable buffer cannot be combined with much, so it only gets what WebGPU allows.
wgpu::BufferUsage complete_usage(wgpu::BufferUsage usage) {
  if (usage & wgpu::BufferUsage::MapRead) {
    return usage | wgpu::BufferUsage::CopyDst;
  }
  if (usage & wgpu::BufferUsage::MapWrite) {
    return usage | wgpu::BufferUsage::CopySrc;
  }
  return usage | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
}

// A range of a buffer that is ready to be mapped: either the buffer itself or a copy of the range
struct MappableRange {
  wgpu::Buffer buffer;
  uint64_t offset; // where the mapping starts in `buffer`
  uint64_t size;   // how much to map
  uint64_t skip;   // bytes between the start of the mapping and the requested data
};

MappableRange make_mappable(const BufferCore& core, uint64_t byte_offset, uint64_t byte_count) {
  // mapping wants an offset that is a multiple of 8 and a size that is a multiple of 4
  const uint64_t begin = byte_offset & ~uint64_t{7};
  const uint64_t end = align_up_4(byte_offset + byte_count);

  if (core.usage & wgpu::BufferUsage::MapRead) {
    return {core.handle, begin, end - begin, byte_offset - begin};
  }

  // not mappable itself: copy the range into a staging buffer that is
  wgpu::BufferDescriptor desc{
      .label = "lab read staging buffer",
      .usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst,
      .size = end - begin,
  };
  wgpu::Buffer staging = core.gpu->device.CreateBuffer(&desc);
  wgpu::CommandEncoder encoder = core.gpu->device.CreateCommandEncoder();
  encoder.CopyBufferToBuffer(core.handle, begin, staging, 0, end - begin);
  wgpu::CommandBuffer commands = encoder.Finish();
  core.gpu->queue.Submit(1, &commands);
  return {staging, 0, end - begin, byte_offset - begin};
}

} // namespace

BufferCore::BufferCore(Gpu& gpu_object, uint64_t size, wgpu::BufferUsage requested_usage, Label name, const void* data,
                       const std::function<void(void* mapped)>& fill)
    : gpu{gpu_object.state()}, usage{complete_usage(requested_usage)}, byte_size{size}, label{std::move(name.text)} {
  const bool initialize = data != nullptr || fill != nullptr;
  wgpu::BufferDescriptor desc{
      .label = std::string_view(label),
      .usage = usage,
      .size = std::max<uint64_t>(align_up_4(byte_size), 4),
      .mappedAtCreation = initialize,
  };
  handle = gpu->device.CreateBuffer(&desc);
  if (!handle) {
    fail(label, std::format("could not create a buffer of {} bytes", byte_size));
  }
  if (initialize) {
    void* mapped = handle.GetMappedRange();
    if (data) {
      std::memcpy(mapped, data, byte_size);
    } else {
      fill(mapped);
    }
    handle.Unmap();
  }
}

void BufferCore::write(uint64_t byte_offset, const void* data, uint64_t byte_count) const {
  if (byte_count == 0) {
    return;
  }
  if (byte_offset % 4 != 0 || byte_count % 4 != 0) {
    // the end of the buffer is padded, so a write that reaches it may be rounded up:
    // the part that is a multiple of 4 bytes goes as it is, the 1 to 3 bytes left over in a padded word
    if (byte_offset % 4 == 0 && byte_offset + byte_count == byte_size) {
      const uint64_t whole = byte_count & ~uint64_t{3};
      if (whole > 0) {
        gpu->queue.WriteBuffer(handle, byte_offset, data, whole);
      }
      char last_word[4] = {};
      std::memcpy(last_word, static_cast<const char*>(data) + whole, byte_count - whole);
      gpu->queue.WriteBuffer(handle, byte_offset + whole, last_word, sizeof(last_word));
      return;
    }
    fail(label, std::format("write: WebGPU writes buffers in multiples of 4 bytes, but this write covers bytes "
                            "[{}, {}). Write an even number of elements.",
                            byte_offset, byte_offset + byte_count));
  }
  gpu->queue.WriteBuffer(handle, byte_offset, data, byte_count);
}

void BufferCore::read(uint64_t byte_offset, void* out, uint64_t byte_count) const {
  if (byte_count == 0) {
    return;
  }
  const MappableRange range = make_mappable(*this, byte_offset, byte_count);

  bool mapped = false;
  gpu->wait(range.buffer.MapAsync(
      wgpu::MapMode::Read, range.offset, range.size, wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::MapAsyncStatus status, wgpu::StringView, bool* mapped) {
        *mapped = status == wgpu::MapAsyncStatus::Success;
      },
      &mapped));
  if (!mapped) {
    fail(label, "read: the buffer could not be mapped");
  }

  const auto* bytes = static_cast<const char*>(range.buffer.GetConstMappedRange(range.offset, range.size));
  std::memcpy(out, bytes + range.skip, byte_count);
  range.buffer.Unmap();
}

void BufferCore::read_async(uint64_t byte_offset, uint64_t byte_count, ReadCallback callback) const {
  if (byte_count == 0) {
    callback(nullptr, 0);
    return;
  }

  // everything the callback needs, kept alive until it has run
  struct Request {
    MappableRange range;
    uint64_t byte_count;
    ReadCallback callback;
    std::string label;
  };
  auto* request = new Request{make_mappable(*this, byte_offset, byte_count), byte_count, std::move(callback), label};

  request->range.buffer.MapAsync(
      wgpu::MapMode::Read, request->range.offset, request->range.size, wgpu::CallbackMode::AllowProcessEvents,
      [](wgpu::MapAsyncStatus status, wgpu::StringView message, Request* raw) {
        std::unique_ptr<Request> request{raw};
        if (status != wgpu::MapAsyncStatus::Success) {
          // also reached when the Gpu is destroyed before the read has finished
          log(LogLevel::warn, "{}: read_async did not finish: {}", request->label, std::string_view(message));
          return;
        }
        const MappableRange& range = request->range;
        const auto* bytes = static_cast<const char*>(range.buffer.GetConstMappedRange(range.offset, range.size));
        request->callback(bytes + range.skip, request->byte_count);
        range.buffer.Unmap();
      },
      request);
}

} // namespace lab::detail
