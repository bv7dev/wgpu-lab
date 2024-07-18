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

  // GLFW Init -----------------------------------------------------------------
  if (!glfwInit()) return 1;
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(WND_WIDTH, WND_HEIGHT, "Hi!", nullptr, nullptr);
  if (!window) {
    std::cerr << "Error: Could not initialize GLFW!" << std::endl;
    glfwTerminate();
    return 1;
  }

  // WGPU Init -----------------------------------------------------------------
  WGPUInstance instance = wgpuCreateInstance(nullptr);
  if (!instance) {
    std::cerr << "Error: Could not initialize WebGPU!" << std::endl;
    return 1;
  }
  std::cout << "WGPU instance: " << instance << std::endl;

  WGPUSurface surface = glfwGetWGPUSurface(instance, window);

  WGPURequestAdapterOptions adapterOpts = {
      .compatibleSurface = surface,
      .powerPreference = WGPUPowerPreference_HighPerformance,
  };
  WGPUAdapter adapter = requestAdapterSync(instance, &adapterOpts);
  std::cout << "Got adapter: " << adapter << std::endl;

  wgpuInstanceRelease(instance);

  inspectAdapter(adapter);

  WGPUDeviceDescriptor deviceDesc = {
      .label = "My device",
      .defaultQueue = {.label = "My default queue"},
      .deviceLostCallbackInfo =
          {
              .mode = WGPUCallbackMode::WGPUCallbackMode_AllowSpontaneous,
              .callback =
                  [](const WGPUDevice* device, WGPUDeviceLostReason reason, char const* message, void*) {
                    std::cout << "Device " << device << " lost: reason " << reason;
                    if (message) std::cout << " (" << message << ")";
                    std::cout << std::endl;
                  },
          },
  };
  WGPUDevice device = requestDeviceSync(adapter, &deviceDesc);
  std::cout << "Got device: " << device << std::endl;

  inspectDevice(device);

  // set device error callback
  auto onDeviceError = [](WGPUErrorType type, char const* message, void*) {
    std::cerr << "Error: Uncaptured device error: type " << type;
    if (message) std::cerr << " (" << message << ")";
    std::cerr << std::endl;
  };
  wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr);

  WGPUSurfaceCapabilities surfaceCapabilities = {};
  wgpuSurfaceGetCapabilities(surface, adapter, &surfaceCapabilities);

  wgpuAdapterRelease(adapter);

  WGPUSurfaceConfiguration surfaceConfig = {
      .device = device,
      .format = surfaceCapabilities.formats[0],
      .usage = WGPUTextureUsage_RenderAttachment,
      .alphaMode = WGPUCompositeAlphaMode_Auto,
      .width = WND_WIDTH,
      .height = WND_HEIGHT,
      .presentMode = WGPUPresentMode_Fifo,
  };
  wgpuSurfaceConfigure(surface, &surfaceConfig);

  WGPUQueue queue = wgpuDeviceGetQueue(device);

  // Main Loop -----------------------------------------------------------------
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    WGPUTextureView targetView = getNextSurfaceTextureView(surface);
    if (!targetView) {
      std::cerr << "Error: Could not get next surface texture view" << std::endl;
      return 1;
    }

    // Create a command encoder for the draw call
    WGPUCommandEncoderDescriptor encoderDesc{.label = "My command encoder"};
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

    // The attachment part of the render pass descriptor describes the target
    // texture of the pass
    WGPURenderPassColorAttachment renderPassColorAttachment = {
        .view = targetView,
        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0},
    };

    // Create the render pass that clears the screen with our color
    WGPURenderPassDescriptor renderPassDesc = {
        .label = "My render pass",
        .colorAttachmentCount = 1,
        .colorAttachments = &renderPassColorAttachment,
    };

    // Create the render pass and end it immediately (we only clear the screen
    // but do not draw anything)
    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);

    // Finally encode and submit the render pass
    WGPUCommandBufferDescriptor cmdBufferDescriptor{.label = "My command buffer"};
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder);

    wgpuQueueSubmit(queue, 1, &command);
    wgpuCommandBufferRelease(command);

    wgpuTextureViewRelease(targetView);
    wgpuSurfacePresent(surface);
    wgpuDeviceTick(device);
  }

  // Cleanup -------------------------------------------------------------------
  wgpuSurfaceUnconfigure(surface);
  wgpuQueueRelease(queue);
  wgpuSurfaceRelease(surface);
  wgpuDeviceRelease(device);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
