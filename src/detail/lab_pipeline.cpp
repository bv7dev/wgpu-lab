#include <objects/lab_pipeline.h>

#include <format>
#include <iostream>
#include <sstream>

namespace lab {

static constexpr uint64_t vertex_format_sizes[32] = {
    0,  // Undefined
    2,  // Uint8x2
    4,  // Uint8x4
    2,  // Sint8x2
    4,  // Sint8x4
    2,  // Unorm8x2
    4,  // Unorm8x4
    2,  // Snorm8x2
    4,  // Snorm8x4
    4,  // Uint16x2
    8,  // Uint16x4
    4,  // Sint16x2
    8,  // Sint16x4
    4,  // Unorm16x2
    8,  // Unorm16x4
    4,  // Snorm16x2
    8,  // Snorm16x4
    4,  // Float16x2
    8,  // Float16x4
    4,  // Float32
    8,  // Float32x2
    12, // Float32x3
    16, // Float32x4
    4,  // Uint32
    8,  // Uint32x2
    12, // Uint32x3
    16, // Uint32x4
    4,  // Sint32
    8,  // Sint32x2
    12, // Sint32x3
    16, // Sint32x4
    4,  // Unorm10_10_10_2
};

constexpr uint64_t vertex_format_size(const wgpu::VertexFormat& format) {
  return vertex_format_sizes[static_cast<size_t>(format)];
}

uint64_t vertex_attributes_stride(const std::vector<wgpu::VertexAttribute>& vertexAttributes) {
  uint64_t totalStride = 0;
  for (const auto& va : vertexAttributes) {
    totalStride += vertex_format_size(va.format);
  }
  return totalStride;
}

wgpu::TextureView get_current_render_texture_view(wgpu::Surface surface) {
  wgpu::SurfaceTexture surfaceTexture;
  surface.GetCurrentTexture(&surfaceTexture);
  if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
    std::cerr << "Error: Pipeline: Could not get current render texture" << std::endl;
    return nullptr;
  }
  wgpu::TextureViewDescriptor viewDescriptor{
      .label = "lab default texture view",
      .format = surfaceTexture.texture.GetFormat(),
      .dimension = wgpu::TextureViewDimension::e2D,
      .baseMipLevel = 0,
      .mipLevelCount = 1,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .aspect = wgpu::TextureAspect::All,
  };
  return surfaceTexture.texture.CreateView(&viewDescriptor);
}

void Pipeline::finalize_config(wgpu::ShaderModule shaderModule) {
  if (label.empty()) {
    label = std::format("Default Pipeline({} on {})", shader.label, webgpu.label);
  }

  for (int i = 0; i < vb_configs.size(); ++i) {
    vb_layouts.push_back({
        .arrayStride = vertex_attributes_stride(vb_configs[i].vertexAttributes),
        .stepMode = vb_configs[i].mode,
        .attributeCount = vb_configs[i].vertexAttributes.size(),
        .attributes = vb_configs[i].vertexAttributes.data(),
    });
  }
  config.vertexState.bufferCount = vb_layouts.size();
  config.vertexState.buffers = vb_layouts.data();

  config.colorTarget.format = webgpu.capabilities.formats[0];
  config.colorTarget.blend = &config.blendState;

  // todo: make number of color targets configurable
  config.fragmentState.targetCount = 1;
  config.fragmentState.targets = &config.colorTarget;

  config.vertexState.module = shaderModule;
  config.fragmentState.module = shaderModule;
}

wgpu::RenderPipeline Pipeline::transfer() const {
  wgpu::PipelineLayout pipelineLayout = nullptr;

  if (bindGroupLayouts.size() > 0) {
    wgpu::PipelineLayoutDescriptor layoutDesc{};

    layoutDesc.bindGroupLayoutCount = bindGroupLayouts.size();
    layoutDesc.bindGroupLayouts = bindGroupLayouts.data();

    pipelineLayout = webgpu.device.CreatePipelineLayout(&layoutDesc);
  }

  wgpu::RenderPipelineDescriptor pipelineDesc = {
      .label = label.c_str(),
      .layout = pipelineLayout,
      .vertex = config.vertexState,
      .primitive = config.primitiveState,
      .multisample = config.multisampleState,
      .fragment = &config.fragmentState,
  };

  return webgpu.device.CreateRenderPipeline(&pipelineDesc);
}

void Pipeline::reset() {
  if (wgpu_pipeline) {
    wgpu_pipeline = nullptr;
  }
}

Pipeline::~Pipeline() { reset(); }

bool Pipeline::default_render(PipelineHandle self, wgpu::Surface surface, const DrawCallParams& draw_params) {
  assert(self->wgpu_pipeline != nullptr);

  wgpu::TextureView targetView = get_current_render_texture_view(surface);

  wgpu::CommandEncoderDescriptor encoderDesc = {.label = "lab default command encoder"};
  wgpu::CommandEncoder encoder = self->webgpu.device.CreateCommandEncoder(&encoderDesc);

  self->render_config.renderPassColorAttachment.view = targetView;

  wgpu::RenderPassDescriptor renderPassDesc = {
      .label = "lab default render pass",
      .colorAttachmentCount = 1,
      .colorAttachments = &self->render_config.renderPassColorAttachment,
  };

  wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDesc);
  renderPass.SetPipeline(self->wgpu_pipeline);

  for (uint32_t i = 0; i < self->vb_configs.size(); ++i) {
    renderPass.SetVertexBuffer(i, self->vb_configs[i].buffer, self->vb_configs[i].offset,
                               self->vb_configs[i].buffer.GetSize());
  }

  for (uint32_t i = 0; i < self->bindGroups.size(); ++i) {
    // TODO: think about how to make use of dynamic offset
    // useful for multiple drawcalls with different uniform data
    renderPass.SetBindGroup(i, self->bindGroups[i], 0, nullptr);
  }

  for (uint32_t i = 0; i < self->ib_configs.size(); ++i) {
    const auto& ibc = self->ib_configs[i];
    renderPass.SetIndexBuffer(ibc.buffer, ibc.format, ibc.offset, ibc.buffer.GetSize());
  }

  // TODO: only a temporary fix (avoid too many branches in render code)
  if (self->ib_configs.size() == 0) {
    renderPass.Draw(draw_params.vertexCount, draw_params.instanceCount, draw_params.firstVertex,
                    draw_params.firstInstance);
  } else {
    renderPass.DrawIndexed(draw_params.vertexCount, draw_params.instanceCount, draw_params.firstVertex);
  }

  renderPass.End();

  wgpu::CommandBufferDescriptor cmdBufferDescriptor = {.label = "lab default command buffer"};
  wgpu::CommandBuffer commands = encoder.Finish(&cmdBufferDescriptor);

  self->webgpu.queue.Submit(1, &commands);
  surface.Present();

  return true;
};

} // namespace lab
