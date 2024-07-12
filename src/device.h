#include <iostream>

#include <webgpu/webgpu.h>

/**
 * Utility function to get a WebGPU device, so that
 *     WGPUAdapter device = requestDeviceSync(adapter, options);
 * is roughly equivalent to
 *     const device = await adapter.requestDevice(descriptor);
 * It is very similar to requestAdapter
 */
WGPUDevice requestDeviceSync(WGPUAdapter adapter,
                             WGPUDeviceDescriptor const* descriptor) {
  struct UserData {
    WGPUDevice device = nullptr;
    bool requestEnded = false;
  } userData;

  auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status,
                                 WGPUDevice device, char const* message,
                                 void* pUserData) {
    UserData& userData = *reinterpret_cast<UserData*>(pUserData);
    if (status == WGPURequestDeviceStatus_Success) {
      userData.device = device;
    } else {
      std::cout << "Error: Could not get WebGPU device: " << message
                << std::endl;
    }
    userData.requestEnded = true;
  };

  wgpuAdapterRequestDevice(adapter, descriptor, onDeviceRequestEnded,
                           (void*)&userData);

  // to add emscripten support later, see requestAdapterSync()

  assert(userData.requestEnded && userData.device != nullptr);
  return userData.device;
}

// We also add an inspect device function:
void inspectDevice(WGPUDevice device) {
  std::vector<WGPUFeatureName> features;
  size_t featureCount = wgpuDeviceEnumerateFeatures(device, nullptr);
  features.resize(featureCount);
  wgpuDeviceEnumerateFeatures(device, features.data());

  std::cout << "Device features:" << std::endl;
  std::cout << std::hex;
  for (auto f : features) {
    std::cout << " - 0x" << f << std::endl;
  }
  std::cout << std::dec;

  WGPUSupportedLimits limits = {};
  limits.nextInChain = nullptr;

  bool success = wgpuDeviceGetLimits(device, &limits) == WGPUStatus_Success;

  if (success) {
    std::cout << "Device limits:" << std::endl;
    std::cout << " - maxTextureDimension1D: "
              << limits.limits.maxTextureDimension1D << std::endl;
    std::cout << " - maxTextureDimension2D: "
              << limits.limits.maxTextureDimension2D << std::endl;
    std::cout << " - maxTextureDimension3D: "
              << limits.limits.maxTextureDimension3D << std::endl;
    std::cout << " - maxTextureArrayLayers: "
              << limits.limits.maxTextureArrayLayers << std::endl;
  }
}
