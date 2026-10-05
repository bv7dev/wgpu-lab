#ifndef WGPU_LAB_COMPUTE_H
#define WGPU_LAB_COMPUTE_H

#include <lab_frame.h>
#include <lab_gpu.h>
#include <lab_pipeline.h>
#include <lab_shader.h>

#include <initializer_list>
#include <string>
#include <vector>

namespace lab {

struct ComputeDesc {
  // only needed if the shader has more than one @compute entry point
  std::string entry = {};

  // Bind group layouts. Leave empty to derive them from the shader (`layout: auto`),
  // which is what `ComputePipeline::bind_group()` builds on.
  std::vector<wgpu::BindGroupLayout> layouts = {};

  // defaults to the label of the shader
  std::string label = {};
};

// A compute shader, ready to run on the gpu.
// ```cpp
// lab::Buffer<float> data(gpu, values, wgpu::BufferUsage::Storage);
// lab::ComputePipeline square(gpu, shader);
// square.run({square.bind_group(0, {{0, data}})}, data.size() / 64);
// std::vector<float> result = data.read();
// ```
class ComputePipeline {
public:
  // Throws lab::Error if WebGPU rejects the pipeline
  ComputePipeline(Gpu& gpu, const Shader& shader, ComputeDesc desc = {});

  ComputePipeline(ComputePipeline&&) = default;
  ComputePipeline& operator=(ComputePipeline&&) = default;

  // Creates the bind group for `@group(group)` of the shader, see Pipeline::bind_group
  wgpu::BindGroup bind_group(uint32_t group, std::initializer_list<Binding> bindings,
                             std::string_view label = {}) const;

  // Runs the shader on x * y * z workgroups and submits that right away
  //  - a workgroup is as many invocations as the shader's @workgroup_size says
  //  - to run it as part of a frame, use `lab::ComputePass`
  void run(std::initializer_list<wgpu::BindGroup> bind_groups, uint32_t x, uint32_t y = 1, uint32_t z = 1) const;

  const wgpu::ComputePipeline& handle() const { return pipeline; }
  const std::string& label() const { return name; }

private:
  std::shared_ptr<detail::GpuState> gpu;
  wgpu::ComputePipeline pipeline;
  std::string name;
};

// A sequence of compute dispatches within a frame. Destroying the pass ends it.
// ```cpp
// lab::Frame frame(gpu);
// {
//   lab::ComputePass compute(frame);
//   compute.dispatch(simulation, {particles_group}, particle_count / 64);
// }
// lab::RenderPass pass(frame, surface);   // renders what was just computed
// pass.draw(pipeline, draw_particles);
// ```
class ComputePass {
public:
  explicit ComputePass(Frame& frame, std::string label = "lab compute pass");

  ComputePass(const ComputePass&) = delete;
  ComputePass& operator=(const ComputePass&) = delete;

  // ends the pass if `end()` was not called
  ~ComputePass();

  void dispatch(const ComputePipeline& pipeline, std::initializer_list<wgpu::BindGroup> bind_groups, uint32_t x,
                uint32_t y = 1, uint32_t z = 1);

  void end();

  const wgpu::ComputePassEncoder& handle() const { return encoder; }

private:
  Frame* frame = nullptr;
  wgpu::ComputePassEncoder encoder;
};

} // namespace lab

#endif // WGPU_LAB_COMPUTE_H
