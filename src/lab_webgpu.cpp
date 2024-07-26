#include "lab_webgpu.h"

#include <glfw3webgpu.h>

namespace lab {

bool init_lab();

Webgpu::Webgpu(const char* lbl) : label{lbl} {
  init_lab();

  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(1, 1, "", nullptr, nullptr);

  wgpu::Surface surface = glfwGetWGPUSurface(wgpu_instance, window);

  wgpu_instance = wgpu::createInstance({});
  if (!wgpu_instance) {
    std::cerr << "Error: WGPU: Could not create Instance!" << std::endl;
    return;
  }
  std::cout << "Info: WGPU: Create: " << wgpu_instance << std::endl;

  wgpu::RequestAdapterOptions adapterOpts = {{
      .compatibleSurface = surface,
      .powerPreference = wgpu::PowerPreference::HighPerformance,
  }};
  wgpu::Adapter adapter = wgpu_instance.requestAdapter(adapterOpts);
  std::cout << "Info: WGPU: Request: " << adapter << std::endl;

  wgpu::DeviceDescriptor deviceDesc = {{
      .label = "default device",
      .defaultQueue = {.label = "default queue"},
      .deviceLostCallbackInfo =
          {
              .mode = wgpu::CallbackMode::AllowSpontaneous,
              .callback =
                  [](const WGPUDevice* device, WGPUDeviceLostReason reason,
                     char const* message, void*) {
                    std::cout << "Warning: WGPU: Device " << device
                              << " lost: reason " << reason;
                    if (message) std::cout << " (" << message << ")";
                    std::cout << std::endl;
                  },
          },
  }};
  device = adapter.requestDevice(deviceDesc);
  std::cout << "Info: WGPU: Request: " << device << std::endl;

  auto onDeviceError = [](wgpu::ErrorType type, char const* message) {
    std::cerr << "Error: WGPU: " << type;
    if (message) std::cerr << " (" << message << ")";
    std::cerr << std::endl;
  };
  auto cb = device.setUncapturedErrorCallback(onDeviceError);

  surface.getCapabilities(adapter, &capabilities);
  adapter.release();

  glfwDestroyWindow(window);

  queue = device.getQueue();
}

Webgpu::~Webgpu() {
  if (wgpu_instance) {
    std::cout << "Info: WGPU: Release: " << wgpu_instance << std::endl;
    queue.release();
    device.release();
    wgpu_instance.release();
    wgpu_instance = nullptr;
  }
}

} // namespace lab
