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
  // bufferDesc.mappedAtCreation = true;
  wgpu::Buffer buffer1 = labgpu.device.createBuffer(bufferDesc);

  // uint8_t* bufferData =
  //     reinterpret_cast<uint8_t*>(buffer1.getMappedRange(0, 16));
  // for (uint8_t i = 0; i < 16; ++i) {
  //   bufferData[i] = 3 + i;
  // }
  // buffer1.unmap();

  // BUFFER 2 (reuse desc)
  bufferDesc.label = "Output buffer";
  bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
  bufferDesc.mappedAtCreation = false;
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
  struct BufferCbUserData1 {
    wgpu::Buffer buffer;
    bool ready;
  } context{buffer1, false};
  wgpu::BufferMapCallbackInfo2 cbinfo;
  cbinfo.callback = [](WGPUMapAsyncStatus status, char const*, void* ud1,
                       void*) {
    std::cout << "Buffer 2 mapped with status " << status << std::endl;
    reinterpret_cast<BufferCbUserData1*>(ud1)->ready = true;
  };
  cbinfo.userdata1 = &context;
  cbinfo.userdata2 = nullptr;
  cbinfo.mode = wgpu::CallbackMode::AllowSpontaneous;
  wgpu::Future fut = buffer2.mapAsync2(wgpu::MapMode::Read, 0, 16, cbinfo);
  //
  // TODO: Investigate: Unfortunately mapAsync gives a warning that it's
  // deprecated and mapAsync2 seems broken or I'm using it wrong?
  // Found issue: dawn crashed because I tried to cout the msg param to see
  // what's it about
  // ---------------------------------------------------------------------------

  // auto cb_lifetime_extender = buffer2.mapAsync(
  //     wgpu::MapMode::Read, 0, 16, [&ready](wgpu::BufferMapAsyncStatus status)
  //     {
  //       std::cout << "Buffer 2 mapped with status: " << status << std::endl;
  //       ready = true;
  //     });

  // tick, so work actually gets performed on device
  while (!context.ready) {
    labgpu.device.tick();
  }

  // Get a pointer to wherever the driver mapped the GPU memory to the RAM
  const uint8_t* bufMap =
      reinterpret_cast<const uint8_t*>(buffer2.getConstMappedRange(0, 16));

  // print mapped data
  for (int i = 0; i < 16; ++i) {
    std::cout << (i > 0 ? ", " : "") << int(bufMap[i]);
  }
  std::cout << std::endl;

  buffer2.unmap();
  buffer2.release();
  buffer1.release();

  std::cout << "\nFIN\n!!!\n" << std::endl;
}
