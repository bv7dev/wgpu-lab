#include <lab>

// This example creates 2 GPU side Buffers, writes data copied from a CPU side
// Buffer into the first GPU Buffer, copies from the first into the second GPU
// Buffer by using a command, and finally it maps the second Buffer back to the
// CPU to print it's content.

// Note: Weirdly, mapAsync2 only works sometimes. No reason discovered so far.
// TODO: investigate further. (probably I'm using it wrong)

int main() {
  lab::Webgpu labgpu{"My Instance"};
  // lab::Window window{"test", 400, 300}; // needed for mapAsync2 ?

  // Buffer 1
  wgpu::BufferDescriptor bufferDesc;
  bufferDesc.label = "My Buffer";
  bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
  bufferDesc.size = 16;
  bufferDesc.mappedAtCreation = false;
  // ---------------------------------------------------------------------------
  // bufferDesc.mappedAtCreation = true;
  wgpu::Buffer buffer1 = labgpu.device.createBuffer(bufferDesc);
  // uint8_t* bufferData =
  //     reinterpret_cast<uint8_t*>(buffer1.getMappedRange(0, 16));
  // for (uint8_t i = 0; i < 16; ++i) {
  //   bufferData[i] = 3 + i;
  // }
  // buffer1.unmap();
  //
  // Alternative to mapAtCreation: writeBuffer (creates copy of data) ----------
  std::vector<uint8_t> numbers(16);
  for (uint8_t i = 0; i < 16; ++i) {
    numbers[i] = i;
  }
  labgpu.queue.writeBuffer(buffer1, 0, numbers.data(), numbers.size());
  // ---------------------------------------------------------------------------

  // Buffer 2 (reuse desc)
  bufferDesc.label = "Output buffer";
  bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
  bufferDesc.mappedAtCreation = false;
  wgpu::Buffer buffer2 = labgpu.device.createBuffer(bufferDesc);

  // Copy one buffer to another on GPU (requires command encoder)
  wgpu::CommandEncoder encoder = labgpu.device.createCommandEncoder({});

  encoder.copyBufferToBuffer(buffer1, 0, buffer2, 0, 16);

  wgpu::CommandBufferDescriptor cmdBufDesc{{.label = "My Command Buffer"}};
  wgpu::CommandBuffer command = encoder.finish(cmdBufDesc);
  encoder.release();

  labgpu.queue.submit(1, &command);
  command.release();

  // doing some ticking here seems to make mapAsync2 work reliably
  // maybe copy command didn't finish (do i need callback?)
  for (int i = 0; i < 1000; ++i) {
    labgpu.device.tick();
    // lab::tick();
  }
  std::cout << "finished ticking\n";

  // Callback to confirm mapping is done
  struct BufferCbUserData1 {
    wgpu::Buffer buffer;
    bool ready;
  } context{buffer2, false};
  wgpu::BufferMapCallbackInfo2 cbinfo{};
  cbinfo.callback = [](WGPUMapAsyncStatus status, char const*, void* ud1,
                       void*) {
    std::cout << "Buffer 2 mapped with status " << status << std::endl;
    BufferCbUserData1* ctx = reinterpret_cast<BufferCbUserData1*>(ud1);
    const uint8_t* bufMap = reinterpret_cast<const uint8_t*>(
        ctx->buffer.getConstMappedRange(0, 16));

    // Print mapped data
    for (int i = 0; i < 16; ++i) {
      std::cout << (i > 0 ? ", " : "") << int(bufMap[i]);
    }
    std::cout << std::endl;
    ctx->buffer.unmap();
    ctx->ready = true;
  };
  cbinfo.userdata1 = &context;
  cbinfo.userdata2 = nullptr;
  cbinfo.mode = wgpu::CallbackMode::AllowSpontaneous;
  wgpu::Future fut = buffer2.mapAsync2(wgpu::MapMode::Read, 0, 16, cbinfo);

  // auto cb = buffer2.mapAsync(wgpu::MapMode::Read, 0, 16,
  //                            [&](WGPUBufferMapAsyncStatus status) {
  //                              if (status !=
  //                              WGPUBufferMapAsyncStatus_Success)
  //                                std::cout << "problem\n";
  //                              context.ready = true;
  //                              std::cout << "map ready\n";
  //                            });

  // Tick, so work actually gets performed on device
  while (!context.ready /* && lab::tick() */) {
    labgpu.device.tick();
  }

  // // Get a pointer to wherever the driver mapped the GPU memory to the RAM
  // const uint8_t* bufMap =
  //     reinterpret_cast<const uint8_t*>(buffer2.getConstMappedRange(0, 16));

  // // Print mapped data
  // for (int i = 0; i < 16; ++i) {
  //   std::cout << (i > 0 ? ", " : "") << int(bufMap[i]);
  // }
  // std::cout << std::endl;

  // buffer2.unmap();

  buffer2.release();
  buffer1.release();

  std::cout << "\nFIN\n!!!\n" << std::endl;
}
