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

  // wgpu::Surface surface = glfwGetWGPUSurface(instance, window);
  wgpu::Surface surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);

  instance = wgpu::CreateInstance({});
  if (!instance) {
    std::cerr << "Error: WGPU: Could not create Instance!" << std::endl;
    return;
  }
  // std::cout << "Info: WGPU: Create: " << instance << std::endl;

  wgpu::RequestAdapterOptions adapterOpts = {
      .compatibleSurface = surface,
      .powerPreference = wgpu::PowerPreference::HighPerformance,
  };
  wgpu::Adapter adapter; // = instance.RequestAdapter(&adapterOpts);
  // typedef void (*WGPURequestAdapterCallback)(WGPURequestAdapterStatus status, WGPUAdapter
  // adapter, char const * message, void * userdata) WGPU_FUNCTION_ATTRIBUTE;
  instance.RequestAdapter(
      &adapterOpts,
      [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message,
         void* userdata) {
        if (message) {
          std::cout << "Request Adapter Cb: " << message << std::endl;
        }
        *(WGPUAdapter*)userdata = adapter;
      },
      &adapter);
  // std::cout << "Info: WGPU: Request: " << adapter << std::endl;

  wgpu::DeviceDescriptor deviceDesc; // = {
  //     .label = "default device",
  //     .defaultQueue = {.label = "default queue"},
  //     .deviceLostCallbackInfo =
  //         {
  //             .mode = wgpu::CallbackMode::AllowSpontaneous,
  //             .callback =
  //                 [](const WGPUDevice* device, WGPUDeviceLostReason reason, char const* message,
  //                    void*) {
  //                   std::cout << "Warning: WGPU: Device " << device << " lost: reason " <<
  //                   reason; if (message) std::cout << " (" << message << ")"; std::cout <<
  //                   std::endl;
  //                 },
  //         },
  // };
  deviceDesc.label = "default device";
  deviceDesc.defaultQueue.label = "default queue";

  wgpu::RequestDeviceCallbackInfo reqCbInfo;
  reqCbInfo.mode = wgpu::CallbackMode::WaitAnyOnly;
  reqCbInfo.callback = [](WGPURequestDeviceStatus status, WGPUDevice device, char const* message,
                          void*) {
    std::cout << "wgpu: Request Device Callback: " << message << std::endl;
  };

  adapter.RequestDevice(
      &deviceDesc,
      [](WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata) {
        if (message) {
          std::cout << "WGPU: Request Device Callback: " << message << std::endl;
        }
        *(WGPUDevice*)userdata = device;
      },
      &device);
  //  (&deviceDesc, deviceDesc.deviceLostCallbackInfo);
  // std::cout << "Info: WGPU: Request: " << device << std::endl;

  // wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, instance);
  device.SetUncapturedErrorCallback(onDeviceError, nullptr);

  surface.GetCapabilities(adapter, &capabilities);
  // adapter.release();

  glfwDestroyWindow(window);

  queue = device.GetQueue();
}

Webgpu::~Webgpu() {
  if (instance) {
    // std::cout << "Info: WGPU: Release: " << instance << std::endl;
    // queue.release();
    // device.release();
    // instance.release();
    instance = nullptr;
  }
}

} // namespace lab
