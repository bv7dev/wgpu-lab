#include <lab>

// this example creates 2 gpu side buffers, writes data copied from a cpu side
// buffer into the first gpu buffer, copies from the first into the second gpu
// buffer by using a command, and finally it maps the second buffer back to the
// cpu to print it's content.

int main() {
  lab::Webgpu labgpu{"My Instance"};

  // BUFFER 1
  wgpu::BufferDescriptor bufferDesc;
  bufferDesc.label = "My Buffer";
  bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
  bufferDesc.size = 16;
  bufferDesc.mappedAtCreation = false;
  wgpu::Buffer buffer1 = labgpu.device.createBuffer(bufferDesc);

  // BUFFER 2 (reuse desc)
  bufferDesc.label = "Output buffer";
  bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
  wgpu::Buffer buffer2 = labgpu.device.createBuffer(bufferDesc);

  // CPU-side data buffer (of size 16 bytes)
  std::vector<uint8_t> numbers(16);
  for (uint8_t i = 0; i < 16; ++i) {
    numbers[i] = i;
  }

  // Copy this from `numbers` (RAM) to `buffer1` (VRAM)
  labgpu.queue.writeBuffer(buffer1, 0, numbers.data(), numbers.size());

  // Copy one buffer to another on GPU (requires command encoder)
  wgpu::CommandEncoder encoder = labgpu.device.createCommandEncoder({});

  encoder.copyBufferToBuffer(buffer1, 0, buffer2, 0, 16);

  wgpu::CommandBuffer command = encoder.finish({});
  encoder.release();
  labgpu.queue.submit(1, &command);
  command.release();

  // Callback to confirm mapping
  struct Context {
    wgpu::Buffer buffer;
    bool ready;
  } context{nullptr, false};

  auto onBuffer2Mapped = [](WGPUBufferMapAsyncStatus status, void* context) {
    std::cout << "Buffer 2 mapped with status " << status << std::endl;
    if (status != wgpu::BufferMapAsyncStatus::Success) {
      std::cout << "Something wrong :(" << std::endl;
      return;
    }
    reinterpret_cast<Context*>(context)->ready = true;
  };
  wgpuBufferMapAsync(buffer2, wgpu::MapMode::Read, 0, 16, onBuffer2Mapped,
                     &context);

  // tick, so work actually gets performed on device
  while (!context.ready) {
    labgpu.device.tick();
  }

  // Get a pointer to wherever the driver mapped the GPU memory to the RAM
  const uint8_t* bufferData =
      reinterpret_cast<const uint8_t*>(buffer2.getConstMappedRange(0, 16));

  // print mapped data
  for (int i = 0; i < 16; ++i) {
    std::cout << (i > 0 ? ", " : "") << int(bufferData[i]);
  }
  std::cout << std::endl;

  buffer2.unmap();
  buffer2.release();
  buffer1.release();

  std::cout << "\nFIN\n!!!\n" << std::endl;
}
