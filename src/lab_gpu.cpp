#include "lab_detail.h"

#include <dawn/webgpu_cpp_print.h>

#include <algorithm>
#include <sstream>

namespace lab {

namespace detail {

namespace {

// webgpu_cpp_print.h can only print to streams
template<typename T>
std::string to_string(const T& value) {
  std::ostringstream stream;
  stream << value;
  return stream.str();
}

} // namespace

GpuState::GpuState() { Runtime::get().gpus.push_back(this); }

GpuState::~GpuState() {
  // release the device before the windows it may still hold swapchains for
  queue = nullptr;
  device = nullptr;
  adapter = nullptr;
  instance = nullptr;

  Runtime& runtime = Runtime::get();
  std::erase(runtime.gpus, this);
  runtime.destroy_retired_windows();
}

void GpuState::wait(wgpu::Future future) const {
  if (instance.WaitAny(future, UINT64_MAX) != wgpu::WaitStatus::Success) {
    fail(label, "waiting for the GPU failed");
  }
}

void GpuState::poll() {
  instance.ProcessEvents();
  if (throw_on_error && errors_thrown < errors.size()) {
    std::string message = errors[errors_thrown];
    if (errors.size() - errors_thrown > 1) {
      message += std::format("\n(and {} more errors, see the log)", errors.size() - errors_thrown - 1);
    }
    errors_thrown = errors.size();
    throw Error(std::format("{}: the GPU reported an error: {}", label, message));
  }
}

std::optional<std::string> capture_error(const GpuState& gpu, const std::function<void()>& create) {
  gpu.device.PushErrorScope(wgpu::ErrorFilter::Validation);
  create();
  std::optional<std::string> error;
  gpu.wait(gpu.device.PopErrorScope(
      wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::PopErrorScopeStatus, wgpu::ErrorType type, wgpu::StringView message, std::optional<std::string>* error) {
        if (type != wgpu::ErrorType::NoError) {
          *error = std::string(std::string_view(message));
        }
      },
      &error));
  return error;
}

} // namespace detail

Gpu::Gpu(GpuOptions options) : shared_state{std::make_shared<detail::GpuState>()} {
  detail::GpuState& state = *shared_state;
  state.label = options.label;
  state.throw_on_error = options.throw_on_error;

  // needed for the blocking waits (creating the device, reading buffers back, ...)
  static const wgpu::InstanceFeatureName instance_features[] = {wgpu::InstanceFeatureName::TimedWaitAny};
  wgpu::InstanceDescriptor instance_desc{
      .requiredFeatureCount = std::size(instance_features),
      .requiredFeatures = instance_features,
  };
  state.instance = wgpu::CreateInstance(&instance_desc);
  if (!state.instance) {
    detail::fail(state.label, "could not create a WebGPU instance");
  }

  // the software adapter is the last resort on machines without a GPU (CI, virtual machines)
  for (bool fallback : {options.fallback_adapter, true}) {
    wgpu::RequestAdapterOptions adapter_options{
        .powerPreference = options.power,
        .forceFallbackAdapter = fallback,
        .backendType = options.backend,
    };
    state.wait(state.instance.RequestAdapter(
        &adapter_options, wgpu::CallbackMode::WaitAnyOnly,
        [](wgpu::RequestAdapterStatus, wgpu::Adapter adapter, wgpu::StringView, wgpu::Adapter* result) {
          *result = std::move(adapter);
        },
        &state.adapter));
    if (state.adapter) {
      break;
    }
  }
  if (!state.adapter) {
    detail::fail(state.label, "no graphics adapter found: neither a GPU nor a software adapter");
  }

  wgpu::AdapterInfo info;
  state.adapter.GetInfo(&info);
  detail::log(LogLevel::info, "{}: using {} ({}, {})", state.label, std::string_view(info.device),
              detail::to_string(info.backendType), detail::to_string(info.adapterType));

  wgpu::DeviceDescriptor device_desc;
  device_desc.label = std::string_view(state.label);
  device_desc.requiredFeatureCount = options.features.size();
  device_desc.requiredFeatures = options.features.data();

  // Both callbacks can run long after this constructor. They only get a raw pointer
  // to the state, which is safe: the state owns the device and so outlives its callbacks.
  device_desc.SetDeviceLostCallback(
      wgpu::CallbackMode::AllowSpontaneous,
      [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message, detail::GpuState* state) {
        // a device that goes away with its Gpu object is business as usual
        if (reason != wgpu::DeviceLostReason::Destroyed && reason != wgpu::DeviceLostReason::CallbackCancelled) {
          detail::log(LogLevel::error, "{}: device lost ({}): {}", state->label, detail::to_string(reason),
                      std::string_view(message));
        }
      },
      &state);
  device_desc.SetUncapturedErrorCallback(
      [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message, detail::GpuState* state) {
        // Thrown later by poll(): this callback is called from inside Dawn,
        // and an exception must not travel through its stack frames.
        std::string text = std::format("{}: {}", detail::to_string(type), std::string_view(message));
        detail::log(LogLevel::error, "{}: {}", state->label, text);
        state->errors.push_back(std::move(text));
      },
      &state);

  struct DeviceRequest {
    wgpu::Device device;
    std::string message;
  } request;
  state.wait(state.adapter.RequestDevice(
      &device_desc, wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::RequestDeviceStatus, wgpu::Device device, wgpu::StringView message, DeviceRequest* request) {
        request->device = std::move(device);
        request->message = std::string(std::string_view(message));
      },
      &request));
  if (!request.device) {
    detail::fail(state.label, std::format("could not create a device: {}", request.message));
  }
  state.device = std::move(request.device);

  state.queue = state.device.GetQueue();
}

const wgpu::Instance& Gpu::instance() const { return shared_state->instance; }
const wgpu::Adapter& Gpu::adapter() const { return shared_state->adapter; }
const wgpu::Device& Gpu::device() const { return shared_state->device; }
const wgpu::Queue& Gpu::queue() const { return shared_state->queue; }

void Gpu::poll() { shared_state->poll(); }

void Gpu::wait_idle() {
  shared_state->wait(shared_state->queue.OnSubmittedWorkDone(wgpu::CallbackMode::WaitAnyOnly,
                                                             [](wgpu::QueueWorkDoneStatus, wgpu::StringView) {}));
}

const std::vector<std::string>& Gpu::errors() const { return shared_state->errors; }

} // namespace lab
