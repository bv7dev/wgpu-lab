#include <iostream>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <webgpu/webgpu.h>

#include "webgpu-utils.h"

int main(int argc, char** argv) {
  constexpr int WND_WIDTH = 640;
  constexpr int WND_HEIGHT = 400;

  std::cout << "Hi!" << std::endl;
  for (int i = 0; i < argc; ++i)
    std::cout << argv[i] << ' ';
  std::cout << std::endl;

  // GLFW Init ----------------------------------------------------------------

  if (!glfwInit()) return 1;
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window =
      glfwCreateWindow(WND_WIDTH, WND_HEIGHT, "Hi!", nullptr, nullptr);
  if (!window) {
    std::cerr << "Error: Could not initialize GLFW!" << std::endl;
    glfwTerminate();
    return 1;
  }

  // WGPU Init ----------------------------------------------------------------

  WGPUInstance instance = wgpuCreateInstance(nullptr);
  if (!instance) {
    std::cerr << "Error: Could not initialize WebGPU!" << std::endl;
    return 1;
  }
  std::cout << "WGPU instance: " << instance << std::endl;

  std::cout << "Requesting adapter..." << std::endl;
  WGPURequestAdapterOptions adapterOpts{};
  WGPUSurface surface = glfwGetWGPUSurface(instance, window);
  adapterOpts.compatibleSurface = surface;
  WGPUAdapter adapter = requestAdapterSync(instance, &adapterOpts);
  wgpuInstanceRelease(instance);
  std::cout << "Got adapter: " << adapter << std::endl;

  inspectAdapter(adapter);

  std::cout << "Requesting device..." << std::endl;
  WGPUDeviceDescriptor deviceDesc{};
  deviceDesc.nextInChain = nullptr;
  deviceDesc.label = "My Device";
  deviceDesc.requiredFeatureCount = 0;
  deviceDesc.requiredLimits = nullptr;
  deviceDesc.defaultQueue.nextInChain = nullptr;
  deviceDesc.defaultQueue.label = "The default queue";
  deviceDesc.deviceLostCallbackInfo = {
      .mode = WGPUCallbackMode::WGPUCallbackMode_AllowSpontaneous,
      .callback =
          [](const WGPUDevice* device, WGPUDeviceLostReason reason,
             char const* message, void*) {
            std::cout << "Device " << device << " lost: reason " << reason;
            if (message) std::cout << " (" << message << ")";
            std::cout << std::endl;
          },
  };
  WGPUDevice device = requestDeviceSync(adapter, &deviceDesc);
  std::cout << "Got device: " << device << std::endl;

  inspectDevice(device);

  // Set breakpoint into onDeviceError() callback for debugging.
  // How to add instance toggle to ensures break will be hit on same callstack:
  // https://eliemichel.github.io/LearnWebGPU/getting-started/adapter-and-device/the-device.html#uncaptured-error-callback
  auto onDeviceError = [](WGPUErrorType type, char const* message, void*) {
    std::cerr << "Error: Uncaptured device error: type " << type;
    if (message) std::cerr << " (" << message << ")";
    std::cerr << std::endl;
  };
  wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr);

  WGPUSurfaceConfiguration surfaceConfig = {};
  surfaceConfig.width = WND_WIDTH;
  surfaceConfig.height = WND_HEIGHT;
  surfaceConfig.usage = WGPUTextureUsage_RenderAttachment;
  WGPUSurfaceCapabilities surfaceCapabilities = {};
  wgpuSurfaceGetCapabilities(surface, adapter, &surfaceCapabilities);
  surfaceConfig.format = surfaceCapabilities.formats[0];
  surfaceConfig.device = device;
  surfaceConfig.presentMode = WGPUPresentMode_Fifo;
  surfaceConfig.alphaMode = WGPUCompositeAlphaMode_Auto;
  wgpuSurfaceConfigure(surface, &surfaceConfig);

  wgpuAdapterRelease(adapter);

  WGPUQueue queue = wgpuDeviceGetQueue(device);

  // Main Loop ----------------------------------------------------------------

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    WGPUTextureView targetView = getNextSurfaceTextureView(surface);
    if (!targetView) return 1;

    // Create a command encoder for the draw call
    WGPUCommandEncoderDescriptor encoderDesc{};
    encoderDesc.nextInChain = nullptr;
    encoderDesc.label = "My command encoder";
    WGPUCommandEncoder encoder =
        wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

    // The attachment part of the render pass descriptor describes the target
    // texture of the pass
    WGPURenderPassColorAttachment renderPassColorAttachment{};
    renderPassColorAttachment.nextInChain = nullptr;
    renderPassColorAttachment.view = targetView;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
    renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
    renderPassColorAttachment.clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0};
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

    // Create the render pass that clears the screen with our color
    WGPURenderPassDescriptor renderPassDesc{};
    renderPassDesc.nextInChain = nullptr;
    renderPassDesc.label = "My render pass";
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;
    renderPassDesc.depthStencilAttachment = nullptr;
    renderPassDesc.timestampWrites = nullptr;

    // Create the render pass and end it immediately (we only clear the screen
    // but do not draw anything)
    WGPURenderPassEncoder renderPass =
        wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);

    // Finally encode and submit the render pass
    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.nextInChain = nullptr;
    cmdBufferDescriptor.label = "Command buffer";
    WGPUCommandBuffer command =
        wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder);

    wgpuQueueSubmit(queue, 1, &command);
    wgpuCommandBufferRelease(command);

    wgpuTextureViewRelease(targetView);
    wgpuSurfacePresent(surface);
    wgpuDeviceTick(device);
  }

  // clean-up
  wgpuSurfaceUnconfigure(surface);
  wgpuQueueRelease(queue);
  wgpuSurfaceRelease(surface);
  wgpuDeviceRelease(device);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
