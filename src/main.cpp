#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#include "webgpu-utils.h"

#include <iostream>

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
  wgpu::Instance instance = wgpu::createInstance({});
  if (!instance) {
    std::cerr << "Error: Could not initialize WebGPU!" << std::endl;
    return 1;
  }
  std::cout << "WGPU instance: " << instance << std::endl;

  wgpu::Surface surface = glfwGetWGPUSurface(instance, window);

  wgpu::RequestAdapterOptions adapterOpts = {{
      .compatibleSurface = surface,
      .powerPreference = wgpu::PowerPreference::HighPerformance,
  }};
  wgpu::Adapter adapter = instance.requestAdapter(adapterOpts);
  std::cout << "Got adapter: " << adapter << std::endl;

  instance.release();

  inspectAdapter(adapter);

  wgpu::DeviceDescriptor deviceDesc = {{
      .label = "My device",
      .defaultQueue = {.label = "My default queue"},
      .deviceLostCallbackInfo =
          {
              .mode = wgpu::CallbackMode::AllowSpontaneous,
              .callback =
                  [](const WGPUDevice* device, WGPUDeviceLostReason reason, char const* message, void*) {
                    std::cout << "Device " << device << " lost: reason " << reason;
                    if (message) std::cout << " (" << message << ")";
                    std::cout << std::endl;
                  },
          },
  }};
  wgpu::Device device = adapter.requestDevice(deviceDesc);
  std::cout << "Got device: " << device << std::endl;

  inspectDevice(device);

  // set device error callback
  auto onDeviceError = [](wgpu::ErrorType type, char const* message) {
    std::cerr << "Error: Uncaptured device error: type " << type;
    if (message) std::cerr << " (" << message << ")";
    std::cerr << std::endl;
  };
  auto cb = device.setUncapturedErrorCallback(onDeviceError);

  wgpu::SurfaceCapabilities surfaceCapabilities;
  surface.getCapabilities(adapter, &surfaceCapabilities);

  adapter.release();

  wgpu::SurfaceConfiguration surfaceConfig = {{
      .device = device,
      .format = surfaceCapabilities.formats[0],
      .usage = wgpu::TextureUsage::RenderAttachment,
      .alphaMode = wgpu::CompositeAlphaMode::Auto,
      .width = WND_WIDTH,
      .height = WND_HEIGHT,
      .presentMode = wgpu::PresentMode::Fifo,
  }};
  surface.configure(surfaceConfig);

  wgpu::Queue queue = device.getQueue();

  // Main Loop -----------------------------------------------------------------
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    wgpu::TextureView targetView = getNextSurfaceTextureView(surface);
    if (!targetView) {
      std::cerr << "Error: Could not get next surface texture view" << std::endl;
      return 1;
    }

    // Create a command encoder for the draw call
    wgpu::CommandEncoderDescriptor encoderDesc = {{.label = "My command encoder"}};
    wgpu::CommandEncoder encoder = device.createCommandEncoder(encoderDesc);

    // The attachment part of the render pass descriptor describes the target
    // texture of the pass
    wgpu::RenderPassColorAttachment renderPassColorAttachment = {{
        .view = targetView,
        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
        .loadOp = wgpu::LoadOp::Clear,
        .storeOp = wgpu::StoreOp::Store,
        .clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0},
    }};

    // Create the render pass that clears the screen with our color
    wgpu::RenderPassDescriptor renderPassDesc = {{
        .label = "My render pass",
        .colorAttachmentCount = 1,
        .colorAttachments = &renderPassColorAttachment,
    }};

    // Create the render pass and end it immediately (we only clear the screen
    // but do not draw anything)
    wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
    renderPass.end();
    renderPass.release();

    // Finally encode and submit the render pass
    wgpu::CommandBufferDescriptor cmdBufferDescriptor = {{.label = "My command buffer"}};
    wgpu::CommandBuffer commands = encoder.finish(cmdBufferDescriptor);
    encoder.release();

    queue.submit(commands);
    commands.release();

    targetView.release();
    surface.present();
    device.tick();
  }

  // Cleanup -------------------------------------------------------------------
  surface.unconfigure();
  queue.release();
  surface.release();
  device.release();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
