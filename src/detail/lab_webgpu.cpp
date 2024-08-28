#include <objects/lab_webgpu.h>

#include <GLFW/glfw3.h>

#include <webgpu/webgpu_glfw.h>

#include <iostream>

namespace lab {

bool init_lab();

void onDeviceError(WGPUErrorType type, char const* message, void* instance) {
  std::cerr << "Error: WGPU(" << instance << "): " << type;
  if (message) std::cerr << " (" << message << ")";
  std::cerr << std::endl;
}

Webgpu::Webgpu(const char* label) : label{label} {
  init_lab();

  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  auto window = glfwCreateWindow(64, 64, "", nullptr, nullptr);
  wgpu::Surface surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);

  wgpu::InstanceDescriptor instanceDesc{.features{.timedWaitAnyEnable{true}}};
  instance = wgpu::CreateInstance(&instanceDesc);
  if (!instance) {
    std::cerr << "Error: WGPU: Could not create Instance!" << std::endl;
    return;
  }

  wgpu::RequestAdapterOptions adapterOpts = {
      .compatibleSurface = surface,
      .powerPreference = wgpu::PowerPreference::HighPerformance,
  };

  wgpu::Future future = instance.RequestAdapter(
      &adapterOpts, wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, const char* message,
         wgpu::Adapter* userdata) {
        if (status == wgpu::RequestAdapterStatus::Success) {
          std::cout << "Info: WGPU: Successfully got adapter!" << std::endl;
        } else {
          std::cerr << "Error: WGPU: Failed to get adapter: ";
          if (message) std::cerr << message;
          std::cerr << std::endl;
        }
        *userdata = std::move(adapter);
      },
      &adapter);
  instance.WaitAny(future, UINT64_MAX);

  wgpu::DeviceDescriptor deviceDesc;
  deviceDesc.label = "lab default device";
  deviceDesc.defaultQueue.label = "lab default queue";
  deviceDesc.deviceLostCallbackInfo.mode = wgpu::CallbackMode::AllowSpontaneous;
  deviceDesc.deviceLostCallbackInfo.callback =
      [](const WGPUDevice* device, WGPUDeviceLostReason reason, char const* message, void*) {
        std::cout << "Info: WGPU: Device " << device << " lost: reason " << reason;
        if (message) std::cout << " (" << message << ")";
        std::cout << std::endl;
      };

  future = adapter.RequestDevice(
      &deviceDesc, wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::RequestDeviceStatus status, wgpu::Device device, char const* message,
         wgpu::Device* userdata) {
        if (status == wgpu::RequestDeviceStatus::Success) {
          std::cout << "Info: WGPU: Successfully got device!" << std::endl;
        } else {
          std::cerr << "Error: WGPU: Failed to get device: ";
          if (message) std::cerr << message;
          std::cerr << std::endl;
        }
        *userdata = std::move(device);
      },
      &device);
  instance.WaitAny(future, UINT64_MAX);

  device.SetUncapturedErrorCallback(onDeviceError, nullptr);

  surface.GetCapabilities(adapter, &capabilities);
  glfwDestroyWindow(window);

  queue = device.GetQueue();
}

Webgpu::~Webgpu() {
  if (instance) {
    instance = nullptr;
    adapter = nullptr;
    device = nullptr;
    queue = nullptr;
  }
}

} // namespace lab
