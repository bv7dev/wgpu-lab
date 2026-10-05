#include "lab_detail.h"

#include <lab_compute.h>

namespace lab {

ComputePipeline::ComputePipeline(Gpu& gpu_object, const Shader& shader, ComputeDesc desc)
    : gpu{gpu_object.state()},
      name{desc.label.empty() ? std::format("compute pipeline({})", shader.label()) : desc.label} {
  wgpu::ComputePipelineDescriptor pipeline_desc;
  pipeline_desc.label = std::string_view(name);
  pipeline_desc.compute.module = shader.handle();
  if (!desc.entry.empty()) {
    pipeline_desc.compute.entryPoint = std::string_view(desc.entry);
  }
  if (!desc.layouts.empty()) {
    wgpu::PipelineLayoutDescriptor layout_desc;
    layout_desc.bindGroupLayoutCount = desc.layouts.size();
    layout_desc.bindGroupLayouts = desc.layouts.data();
    pipeline_desc.layout = gpu->device.CreatePipelineLayout(&layout_desc);
  }

  if (auto error = detail::capture_error(*gpu, [&] { pipeline = gpu->device.CreateComputePipeline(&pipeline_desc); })) {
    detail::fail(name, *error);
  }
}

wgpu::BindGroup ComputePipeline::bind_group(uint32_t group, std::initializer_list<Binding> bindings,
                                            std::string_view label) const {
  const std::string group_label = label.empty() ? std::format("{} bind group {}", name, group) : std::string{label};

  wgpu::BindGroupLayout layout;
  if (auto error = detail::capture_error(*gpu, [&] { layout = pipeline.GetBindGroupLayout(group); })) {
    detail::fail(group_label, *error);
  }
  return detail::make_bind_group(gpu, layout, bindings, group_label);
}

void ComputePipeline::run(std::initializer_list<wgpu::BindGroup> bind_groups, uint32_t x, uint32_t y,
                          uint32_t z) const {
  wgpu::CommandEncoder encoder = gpu->device.CreateCommandEncoder();
  wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
  pass.SetPipeline(pipeline);
  uint32_t index = 0;
  for (const wgpu::BindGroup& group : bind_groups) {
    pass.SetBindGroup(index++, group);
  }
  pass.DispatchWorkgroups(x, y, z);
  pass.End();
  wgpu::CommandBuffer commands = encoder.Finish();
  gpu->queue.Submit(1, &commands);
}

ComputePass::ComputePass(Frame& target_frame, std::string label) : frame{&target_frame} {
  if (frame->submitted) {
    detail::fail(label, "the frame of this pass has already been submitted");
  }
  if (frame->pass_open) {
    detail::fail(label, "another pass of the same frame is still open, end it first");
  }
  wgpu::ComputePassDescriptor desc;
  desc.label = std::string_view(label);
  encoder = frame->encoder.BeginComputePass(&desc);
  frame->pass_open = true;
}

ComputePass::~ComputePass() { end(); }

void ComputePass::end() {
  if (encoder) {
    encoder.End();
    encoder = nullptr;
    frame->pass_open = false;
  }
}

void ComputePass::dispatch(const ComputePipeline& pipeline, std::initializer_list<wgpu::BindGroup> bind_groups,
                           uint32_t x, uint32_t y, uint32_t z) {
  if (!encoder) {
    detail::fail(pipeline.label(), "dispatch: the compute pass has already ended");
  }
  encoder.SetPipeline(pipeline.handle());
  uint32_t index = 0;
  for (const wgpu::BindGroup& group : bind_groups) {
    encoder.SetBindGroup(index++, group);
  }
  encoder.DispatchWorkgroups(x, y, z);
}

} // namespace lab
