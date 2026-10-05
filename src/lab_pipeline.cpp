#include "lab_detail.h"

#include <lab_frame.h>
#include <lab_pipeline.h>

#include <webgpu/webgpu_cpp_print.h>

#include <algorithm>
#include <sstream>

namespace lab {

namespace detail {

VertexLayout make_vertex_layout(std::initializer_list<VertexAttribute> attributes, uint64_t stride,
                                wgpu::VertexStepMode step_mode) {
  VertexLayout layout{.attributes = attributes, .stride = stride, .step_mode = step_mode};

  // attributes without an offset follow the one before them
  uint64_t next_offset = 0;
  for (VertexAttribute& attribute : layout.attributes) {
    if (attribute.offset == VertexAttribute::next_offset) {
      attribute.offset = next_offset;
    }
    next_offset = attribute.offset + vertex_format_size(attribute.format);
    if (next_offset > stride) {
      fail("vertex layout", std::format("the attributes reach byte {} of a vertex, but the vertex type has only "
                                        "{} bytes: do the formats match the members of the type?",
                                        next_offset, stride));
    }
  }
  return layout;
}

wgpu::BindGroup make_bind_group(const std::shared_ptr<GpuState>& gpu, const wgpu::BindGroupLayout& layout,
                                std::initializer_list<Binding> bindings, std::string_view label) {
  std::vector<wgpu::BindGroupEntry> entries;
  entries.reserve(bindings.size());
  for (const Binding& binding : bindings) {
    wgpu::BindGroupEntry entry;
    entry.binding = binding.binding;
    entry.buffer = binding.buffer;
    entry.offset = binding.offset;
    entry.size = binding.size;
    entry.sampler = binding.sampler;
    entry.textureView = binding.texture_view;
    entries.push_back(entry);
  }

  wgpu::BindGroupDescriptor desc;
  desc.label = label;
  desc.layout = layout;
  desc.entryCount = entries.size();
  desc.entries = entries.data();

  wgpu::BindGroup group;
  if (auto error = capture_error(*gpu, [&] { group = gpu->device.CreateBindGroup(&desc); })) {
    fail(label, std::format("{}\nNote: a binding that the shader declares but does not use is not part of the "
                            "layout WebGPU derives from the shader.",
                            *error));
  }
  return group;
}

} // namespace detail

Pipeline::Pipeline(Gpu& gpu_object, const Shader& shader, PipelineDesc desc)
    : gpu{gpu_object.state()}, target_format{desc.target},
      name{desc.label.empty() ? std::format("pipeline({})", shader.label()) : desc.label} {
  // vertex buffers: shader locations count up across all slots unless they are given
  std::vector<std::vector<wgpu::VertexAttribute>> attributes(desc.vertex_buffers.size());
  std::vector<wgpu::VertexBufferLayout> buffer_layouts;
  uint32_t next_location = 0;
  for (size_t slot = 0; slot < desc.vertex_buffers.size(); ++slot) {
    const VertexLayout& layout = desc.vertex_buffers[slot];
    for (const VertexAttribute& attribute : layout.attributes) {
      const uint32_t location =
          attribute.location == VertexAttribute::next_location ? next_location : attribute.location;
      next_location = location + 1;
      attributes[slot].push_back({.format = attribute.format, .offset = attribute.offset, .shaderLocation = location});
    }
    buffer_layouts.push_back({
        .stepMode = layout.step_mode,
        .arrayStride = layout.stride,
        .attributeCount = attributes[slot].size(),
        .attributes = attributes[slot].data(),
    });
    vertex_slots.push_back({layout.stride, layout.step_mode});
  }

  wgpu::ColorTargetState color_target;
  color_target.format = desc.target.color;
  color_target.blend = desc.blend ? &*desc.blend : nullptr;

  wgpu::FragmentState fragment;
  fragment.module = shader.handle();
  fragment.targetCount = 1;
  fragment.targets = &color_target;
  if (!desc.fragment_entry.empty()) {
    fragment.entryPoint = std::string_view(desc.fragment_entry);
  }

  wgpu::DepthStencilState depth_stencil;
  depth_stencil.format = desc.target.depth;
  depth_stencil.depthWriteEnabled = wgpu::OptionalBool::True;
  depth_stencil.depthCompare = wgpu::CompareFunction::Less;

  wgpu::RenderPipelineDescriptor pipeline_desc;
  pipeline_desc.label = std::string_view(name);
  pipeline_desc.vertex.module = shader.handle();
  pipeline_desc.vertex.bufferCount = buffer_layouts.size();
  pipeline_desc.vertex.buffers = buffer_layouts.data();
  if (!desc.vertex_entry.empty()) {
    pipeline_desc.vertex.entryPoint = std::string_view(desc.vertex_entry);
  }
  pipeline_desc.primitive.topology = desc.topology;
  pipeline_desc.primitive.cullMode = desc.cull;
  pipeline_desc.fragment = &fragment;
  if (desc.target.depth != wgpu::TextureFormat::Undefined) {
    pipeline_desc.depthStencil = &depth_stencil;
  }
  if (!desc.layouts.empty()) {
    wgpu::PipelineLayoutDescriptor layout_desc;
    layout_desc.bindGroupLayoutCount = desc.layouts.size();
    layout_desc.bindGroupLayouts = desc.layouts.data();
    pipeline_desc.layout = gpu->device.CreatePipelineLayout(&layout_desc);
  }
  if (desc.customize) {
    desc.customize(pipeline_desc);
  }

  if (auto error = detail::capture_error(*gpu, [&] { pipeline = gpu->device.CreateRenderPipeline(&pipeline_desc); })) {
    detail::fail(name, *error);
  }
}

wgpu::BindGroup Pipeline::bind_group(uint32_t group, std::initializer_list<Binding> bindings,
                                     std::string_view label) const {
  const std::string group_label = label.empty() ? std::format("{} bind group {}", name, group) : std::string{label};

  wgpu::BindGroupLayout layout;
  if (auto error = detail::capture_error(*gpu, [&] { layout = pipeline.GetBindGroupLayout(group); })) {
    detail::fail(group_label, *error);
  }
  return detail::make_bind_group(gpu, layout, bindings, group_label);
}

bool Pipeline::render_frame(Surface& surface, const Draw& draw) const {
  RenderPass pass(surface);
  pass.draw(*this, draw);
  return static_cast<bool>(pass);
}

bool Pipeline::render_frame(Surface& surface, uint32_t vertex_count, uint32_t instance_count) const {
  return render_frame(surface, Draw{.count = vertex_count, .instances = instance_count});
}

} // namespace lab
