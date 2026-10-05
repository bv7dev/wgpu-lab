#ifndef WGPU_LAB_GPU_H
#define WGPU_LAB_GPU_H

#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <string>
#include <vector>

namespace lab {

namespace detail {
struct GpuState;
}

struct GpuOptions {
  // which GPU to prefer if there is more than one
  wgpu::PowerPreference power = wgpu::PowerPreference::HighPerformance;

  // `Undefined` leaves the choice to Dawn (Vulkan on Linux, D3D12 on Windows)
  wgpu::BackendType backend = wgpu::BackendType::Undefined;

  // use the software adapter even if there is a GPU
  //  - if there is no GPU at all, the software adapter is used anyway
  bool fallback_adapter = false;

  // device features the program cannot do without, creating the Gpu fails if one is missing
  std::vector<wgpu::FeatureName> features = {};

  // An error reported by the GPU (failed validation, out of memory) is thrown as
  // lab::Error by the next `tick()` or `poll()`.
  //  - if false, errors are only logged and collected in `errors()`
  bool throw_on_error = true;

  std::string label = "lab";
};

// The connection to the graphics card: WebGPU instance, adapter, device and queue.
// Everything else that lives on the GPU is created from a Gpu.
// ```cpp
// lab::Gpu gpu;
// wgpu::Device device = gpu.device(); // plain WebGPU is always within reach
// ```
class Gpu {
public:
  // Throws lab::Error if no adapter or device can be created
  explicit Gpu(GpuOptions options = {});

  Gpu(Gpu&&) = default;
  Gpu& operator=(Gpu&&) = default;

  const wgpu::Instance& instance() const;
  const wgpu::Adapter& adapter() const;
  const wgpu::Device& device() const;
  const wgpu::Queue& queue() const;

  // Runs pending WebGPU callbacks and throws lab::Error if the GPU has reported
  // an error since the last call (see GpuOptions::throw_on_error)
  //  - `lab::tick()` does this for every Gpu, so only programs without
  //    a main loop need to call it themselves
  void poll();

  // Blocks until the GPU has executed everything that was submitted so far
  void wait_idle();

  // Messages of all errors the GPU has reported so far
  const std::vector<std::string>& errors() const;

  // internal: shared with every object that is created from this Gpu
  const std::shared_ptr<detail::GpuState>& state() const { return shared_state; }

private:
  std::shared_ptr<detail::GpuState> shared_state;
};

} // namespace lab

#endif // WGPU_LAB_GPU_H
