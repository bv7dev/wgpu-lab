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

  WGPUInstanceDescriptor desc = {};
  // Toggle used to make error callback immediate, so that breakpoint hits on
  // same callstack. Set breakpoint into onDeviceError() callback for debugging.
  // https://eliemichel.github.io/LearnWebGPU/getting-started/adapter-and-device/the-device.html#uncaptured-error-callback
  // WGPUDawnTogglesDescriptor toggles;
  // toggles.chain.next = nullptr;
  // toggles.chain.sType = WGPUSType_DawnTogglesDescriptor;
  // toggles.disabledToggleCount = 0;
  // toggles.enabledToggleCount = 1;
  // const char* toggleName = "enable_immediate_error_handling";
  // toggles.enabledToggles = &toggleName;
  // desc.nextInChain = &toggles.chain;

  // Toggle seems to make no difference.
  // Tested with error produced by commenting out line:
  // renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

  WGPUInstance instance = wgpuCreateInstance(&desc);
  if (!instance) {
    std::cerr << "Error: Could not initialize WebGPU!" << std::endl;
    return 1;
  }
  std::cout << "WGPU instance: " << instance << std::endl;

  std::cout << "Requesting adapter..." << std::endl;
  WGPURequestAdapterOptions adapterOpts = {};
  WGPUSurface surface = glfwGetWGPUSurface(instance, window);
  adapterOpts.compatibleSurface = surface;
  WGPUAdapter adapter = requestAdapterSync(instance, &adapterOpts);
  std::cout << "Got adapter: " << adapter << std::endl;
  wgpuInstanceRelease(instance);
  inspectAdapter(adapter);

  std::cout << "Requesting device..." << std::endl;
  WGPUDeviceDescriptor deviceDesc = {};
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
             char const* message, void* /* pUserData */) {
            std::cout << "Device " << device << " lost: reason " << reason;
            if (message) std::cout << " (" << message << ")";
            std::cout << std::endl;
          },
  };
  WGPUDevice device = requestDeviceSync(adapter, &deviceDesc);
  std::cout << "Got device: " << device << std::endl;
  inspectDevice(device);

  // Set breakpoint into onDeviceError() callback for debugging.
  // Instance toggle ensures that it will break on the same callstack.
  auto onDeviceError = [](WGPUErrorType type, char const* message,
                          void* /* pUserData */) {
    std::cerr << "Error: Uncaptured device error: type " << type;
    if (message) std::cerr << " (" << message << ")";
    std::cerr << std::endl;
  };
  wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError,
                                       nullptr /* pUserData */);

  WGPUQueue queue = wgpuDeviceGetQueue(device);

  // auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status,
  //                           void* /* pUserData */) {
  //   std::cout << "Queued work finished with status: " << status << std::endl;
  // };
  // TODO: Warning: Old OnSubmittedWorkDone APIs are deprecated. If using C
  // please pass a CallbackInfo struct that has two userdatas. Otherwise, if
  // using C++, please use templated helpers.
  // wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, nullptr /* pUserData
  // */);

  WGPUQueueWorkDoneCallbackInfo2 workSubmittedInfo{};
  workSubmittedInfo.callback = [](WGPUQueueWorkDoneStatus status, void*,
                                  void*) {
    std::cout << "Queued work finished with status: " << status << std::endl;
  };
  workSubmittedInfo.mode =
      WGPUCallbackMode::WGPUCallbackMode_AllowProcessEvents;
  wgpuQueueOnSubmittedWorkDone2(queue, workSubmittedInfo);
  // Don't know what above callback is supposed to do. Will remove it next.

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

  // glfw main loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    WGPUTextureView targetView = getNextSurfaceTextureView(surface);
    if (!targetView) return 1;

    // Create a command encoder for the draw call
    WGPUCommandEncoderDescriptor encoderDesc{};
    encoderDesc.label = "My command encoder";
    WGPUCommandEncoder encoder =
        wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

    // The attachment part of the render pass descriptor describes the target
    // texture of the pass
    WGPURenderPassColorAttachment renderPassColorAttachment{};
    renderPassColorAttachment.view = targetView;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
    renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
    renderPassColorAttachment.clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0};
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

    // Create the render pass that clears the screen with our color
    WGPURenderPassDescriptor renderPassDesc{};
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
    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.label = "Command buffer";
    WGPUCommandBuffer command =
        wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder);

    std::cout << "Submitting command..." << std::endl;
    wgpuQueueSubmit(queue, 1, &command);
    wgpuCommandBufferRelease(command);
    std::cout << "Command submitted." << std::endl;

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
