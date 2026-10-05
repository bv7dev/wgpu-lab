#include <objects/lab_webgpu.h>

#include <webgpu/webgpu_cpp_print.h>

#include <iostream>
#include <string_view>

namespace lab {

Webgpu::Webgpu(const std::string& lbl, wgpu::PowerPreference power_pref) : label{lbl} {
  // needed for the blocking instance.WaitAny() calls below
  static const wgpu::InstanceFeatureName required_features[] = {wgpu::InstanceFeatureName::TimedWaitAny};
  wgpu::InstanceDescriptor instanceDesc{
      .requiredFeatureCount = std::size(required_features),
      .requiredFeatures = required_features,
  };
  instance = wgpu::CreateInstance(&instanceDesc);
  if (!instance) {
    std::cerr << "Error: WGPU: Could not create Instance!" << std::endl;
    return;
  }

  wgpu::RequestAdapterOptions adapterOpts = {
      .powerPreference = power_pref,
  };

  wgpu::Future future = instance.RequestAdapter(
      &adapterOpts, wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message, wgpu::Adapter* userdata) {
        if (status == wgpu::RequestAdapterStatus::Success) {
          std::cout << "Info: WGPU: Successfully got adapter!" << std::endl;
        } else {
          std::cerr << "Error: WGPU: Failed to get adapter: " << std::string_view(message) << std::endl;
        }
        *userdata = std::move(adapter);
      },
      &adapter);
  instance.WaitAny(future, UINT64_MAX);
  if (!adapter) {
    return;
  }

  wgpu::DeviceDescriptor deviceDesc;
  deviceDesc.label = "lab default device";
  deviceDesc.defaultQueue.label = "lab default queue";
  deviceDesc.SetDeviceLostCallback(
      wgpu::CallbackMode::AllowSpontaneous,
      [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
        // a device that is destroyed along with its Webgpu object is not worth reporting
        if (reason == wgpu::DeviceLostReason::Destroyed || reason == wgpu::DeviceLostReason::CallbackCancelled) {
          return;
        }
        std::cerr << "Error: WGPU: Device lost: " << reason << " (" << std::string_view(message) << ")" << std::endl;
      });
  deviceDesc.SetUncapturedErrorCallback([](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
    std::cerr << "Error: WGPU: " << type << ": " << std::string_view(message) << std::endl;
  });

  future = adapter.RequestDevice(
      &deviceDesc, wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView message, wgpu::Device* userdata) {
        if (status == wgpu::RequestDeviceStatus::Success) {
          std::cout << "Info: WGPU: Successfully got device!" << std::endl;
        } else {
          std::cerr << "Error: WGPU: Failed to get device: " << std::string_view(message) << std::endl;
        }
        *userdata = std::move(device);
      },
      &device);
  instance.WaitAny(future, UINT64_MAX);
  if (!device) {
    return;
  }

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
