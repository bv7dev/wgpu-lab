#include <objects/lab_webgpu.h>

#include <glfw3webgpu.h>

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

  wgpu::Surface surface = glfwGetWGPUSurface(instance, window);

  instance = wgpu::createInstance({});
  if (!instance) {
    std::cerr << "Error: WGPU: Could not create Instance!" << std::endl;
    return;
  }
  std::cout << "Info: WGPU: Create: " << instance << std::endl;

  wgpu::RequestAdapterOptions adapterOpts = {{
      .compatibleSurface = surface,
      .powerPreference = wgpu::PowerPreference::HighPerformance,
  }};
  wgpu::Adapter adapter = instance.requestAdapter(adapterOpts);
  std::cout << "Info: WGPU: Request: " << adapter << std::endl;

  wgpu::DeviceDescriptor deviceDesc = {{
      .label = "default device",
      .defaultQueue = {.label = "default queue"},
      .deviceLostCallbackInfo =
          {
              .mode = wgpu::CallbackMode::AllowSpontaneous,
              .callback =
                  [](const WGPUDevice* device, WGPUDeviceLostReason reason, char const* message,
                     void*) {
                    std::cout << "Warning: WGPU: Device " << device << " lost: reason " << reason;
                    if (message) std::cout << " (" << message << ")";
                    std::cout << std::endl;
                  },
          },
  }};
  device = adapter.requestDevice(deviceDesc);
  std::cout << "Info: WGPU: Request: " << device << std::endl;

  wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, instance);

  surface.getCapabilities(adapter, &capabilities);
  adapter.release();

  glfwDestroyWindow(window);

  queue = device.getQueue();
}

Webgpu::~Webgpu() {
  if (instance) {
    std::cout << "Info: WGPU: Release: " << instance << std::endl;
    queue.release();
    device.release();
    instance.release();
    instance = nullptr;
  }
}

} // namespace lab
