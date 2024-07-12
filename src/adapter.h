#include <cassert>
#include <iostream>
#include <vector>

#include <webgpu/webgpu.h>

/**
 * Utility function to get a WebGPU adapter, so that
 *     WGPUAdapter adapter = requestAdapterSync(options);
 * is roughly equivalent to
 *     const adapter = await navigator.gpu.requestAdapter(options);
 */
WGPUAdapter requestAdapterSync(WGPUInstance instance,
                               WGPURequestAdapterOptions const* options) {
  // A simple structure holding the local information shared with the
  // onAdapterRequestEnded callback.
  struct UserData {
    WGPUAdapter adapter = nullptr;
    bool requestEnded = false;
  } userData;

  // Callback called by wgpuInstanceRequestAdapter when the request returns
  // This is a C++ lambda function, but could be any function defined in the
  // global scope. It must be non-capturing (the brackets [] are empty) so
  // that it behaves like a regular C function pointer, which is what
  // wgpuInstanceRequestAdapter expects (WebGPU being a C API). The workaround
  // is to convey what we want to capture through the pUserData pointer,
  // provided as the last argument of wgpuInstanceRequestAdapter and received
  // by the callback as its last argument.
  auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status,
                                  WGPUAdapter adapter, char const* message,
                                  void* pUserData) {
    UserData& userData = *reinterpret_cast<UserData*>(pUserData);
    if (status == WGPURequestAdapterStatus_Success) {
      userData.adapter = adapter;
    } else {
      std::cerr << "Error: Could not get WebGPU adapter: " << message
                << std::endl;
    }
    userData.requestEnded = true;
  };

  // Call to the WebGPU request adapter procedure
  wgpuInstanceRequestAdapter(instance /* equivalent of navigator.gpu */,
                             options, onAdapterRequestEnded, (void*)&userData);

  // We wait until userData.requestEnded gets true.
  // Waiting only needed when using emscripten. See:
  // https://eliemichel.github.io/LearnWebGPU/getting-started/adapter-and-device/the-adapter.html

  assert(userData.requestEnded && userData.adapter != nullptr);
  return userData.adapter;
}

void inspectAdapter(WGPUAdapter adapter) {
  WGPUSupportedLimits supportedLimits = {};
  supportedLimits.nextInChain = nullptr;

  bool success =
      wgpuAdapterGetLimits(adapter, &supportedLimits) == WGPUStatus_Success;

  if (success) {
    std::cout << "Adapter limits:" << std::endl;
    std::cout << " - maxTextureDimension1D: "
              << supportedLimits.limits.maxTextureDimension1D << std::endl;
    std::cout << " - maxTextureDimension2D: "
              << supportedLimits.limits.maxTextureDimension2D << std::endl;
    std::cout << " - maxTextureDimension3D: "
              << supportedLimits.limits.maxTextureDimension3D << std::endl;
    std::cout << " - maxTextureArrayLayers: "
              << supportedLimits.limits.maxTextureArrayLayers << std::endl;
  }
  std::vector<WGPUFeatureName> features;

  // Call the function a first time with a null return address, just to get
  // the entry count.
  size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, nullptr);

  // Allocate memory (could be a new, or a malloc() if this were a C program)
  features.resize(featureCount);

  // Call the function a second time, with a non-null return address
  wgpuAdapterEnumerateFeatures(adapter, features.data());

  std::cout << "Adapter features:" << std::endl;
  std::cout << std::hex; // Write integers as hexadecimal to ease comparison
                         // with webgpu.h literals
  for (auto f : features) {
    std::cout << " - 0x" << f << std::endl;
  }
  std::cout << std::dec; // Restore decimal numbers
  WGPUAdapterProperties properties = {};
  properties.nextInChain = nullptr;
  wgpuAdapterGetProperties(adapter, &properties);
  std::cout << "Adapter properties:" << std::endl;
  std::cout << " - vendorID: " << properties.vendorID << std::endl;
  if (properties.vendorName) {
    std::cout << " - vendorName: " << properties.vendorName << std::endl;
  }
  if (properties.architecture) {
    std::cout << " - architecture: " << properties.architecture << std::endl;
  }
  std::cout << " - deviceID: " << properties.deviceID << std::endl;
  if (properties.name) {
    std::cout << " - name: " << properties.name << std::endl;
  }
  if (properties.driverDescription) {
    std::cout << " - driverDescription: " << properties.driverDescription
              << std::endl;
  }
  std::cout << std::hex;
  std::cout << " - adapterType: 0x" << properties.adapterType << std::endl;
  std::cout << " - backendType: 0x" << properties.backendType << std::endl;
  std::cout << std::dec; // Restore decimal numbers
}
