#include <iostream>

#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>

#include "adapter.h"
#include "device.h"

int main(int argc, char** argv) {
  std::cout << "Hi!" << std::endl;
  for (int i = 0; i < argc; ++i)
    std::cout << argv[i] << ' ';
  std::cout << std::endl;

  if (!glfwInit()) return 1;
  GLFWwindow* window = glfwCreateWindow(640, 400, "Hi!", NULL, NULL);
  if (!window) {
    std::cerr << "Error: Could not initialize GLFW!" << std::endl;
    glfwTerminate();
    return 1;
  }

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
  glfwTerminate();

  WGPUInstanceDescriptor desc = {};
  // Toggle used to make error callback immediate, so that breakpoint hits on
  // same callstack. Set breakpoint into onDeviceError() callback for debugging.
  // https://eliemichel.github.io/LearnWebGPU/getting-started/adapter-and-device/the-device.html#uncaptured-error-callback
  WGPUDawnTogglesDescriptor toggles;
  toggles.chain.next = nullptr;
  toggles.chain.sType = WGPUSType_DawnTogglesDescriptor;
  toggles.disabledToggleCount = 0;
  toggles.enabledToggleCount = 1;
  const char* toggleName = "enable_immediate_error_handling";
  toggles.enabledToggles = &toggleName;
  desc.nextInChain = &toggles.chain;
  WGPUInstance instance = wgpuCreateInstance(&desc);
  if (!instance) {
    std::cerr << "Error: Could not initialize WebGPU!" << std::endl;
    return 1;
  }
  std::cout << "WGPU instance: " << instance << std::endl;

  std::cout << "Requesting adapter..." << std::endl;
  WGPURequestAdapterOptions adapterOpts = {};
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
  wgpuAdapterRelease(adapter);
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
  wgpuDeviceRelease(device);

  return 0;
}
