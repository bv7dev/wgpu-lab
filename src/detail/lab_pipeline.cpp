#include <objects/lab_pipeline.h>

#include <format>
#include <iostream>
#include <sstream>

namespace lab {

void Pipeline::finalize_config(wgpu::ShaderModule shaderModule) {
  if (label.empty()) {
    label = std::format("Default Pipeline({} on {})", shader.label, webgpu.label);
  }

  for (int i = 0; i < vb_configs.size(); ++i) {
    vb_layouts.push_back({{
        .arrayStride = get_total_stride(vb_configs[i].vertexAttributes),
        .stepMode = vb_configs[i].mode,
        .attributeCount = vb_configs[i].vertexAttributes.size(),
        .attributes = vb_configs[i].vertexAttributes.data(),
    }});
  }
  config.vertexState.bufferCount = vb_layouts.size();
  config.vertexState.buffers = vb_layouts.size() > 0 ? vb_layouts.data() : nullptr;

  config.colorTarget.format = webgpu.capabilities.formats[0];
  config.colorTarget.blend = &config.blendState;

  // todo: make number of color targets configurable
  config.fragmentState.targetCount = 1;
  config.fragmentState.targets = &config.colorTarget;

  config.vertexState.module = shaderModule;
  config.fragmentState.module = shaderModule;

  // WIP: Bind layout and group for uniform buffers

  // Create binding layout (don't forget to = Default)
  bindingLayout.binding = 0;
  bindingLayout.visibility = wgpu::ShaderStage::Vertex;
  bindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
  bindingLayout.buffer.minBindingSize = sizeof(float);

  // Create a bind group layout
  bindGroupLayoutDesc.entryCount = 1;
  bindGroupLayoutDesc.entries = &bindingLayout;
  bindGroupLayout = webgpu.device.createBindGroupLayout(bindGroupLayoutDesc);

  // Create the pipeline layout
  layoutDesc.bindGroupLayoutCount = 1;
  layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;

  binding.binding = 0;

  // The buffer it is actually bound to
  // binding.buffer = uniformBuffer;

  // We can specify an offset within the buffer, so that a single buffer can hold
  // multiple uniform blocks.
  binding.offset = 0;

  // And we specify again the size of the buffer.
  binding.size = sizeof(float);

  // A bind group contains one or multiple bindings
  bindGroupDesc.layout = bindGroupLayout;
  bindGroupDesc.entryCount = bindGroupLayoutDesc.entryCount;
  bindGroupDesc.entries = &binding;

  bindGroup = webgpu.device.createBindGroup(bindGroupDesc);
}

wgpu::RenderPipeline Pipeline::transfer() const {
  wgpu::RenderPipelineDescriptor pipelineDesc = {{
      .label = label.c_str(),
      .layout = webgpu.device.createPipelineLayout(layoutDesc),
      .vertex = config.vertexState,
      .primitive = config.primitiveState,
      .multisample = config.multisampleState,
      .fragment = &config.fragmentState,
  }};

  return webgpu.device.createRenderPipeline(pipelineDesc);
}

Pipeline::~Pipeline() {
  if (wgpu_pipeline) {
    wgpu_pipeline.release();
    wgpu_pipeline = nullptr;
  }
}

bool Pipeline::default_render(PipelineHandle self, wgpu::Surface surface,
                              const DrawCallParams& draw_params) {
  wgpu::TextureView targetView = get_target_view(surface);
  if (!targetView) {
    std::cerr << "Error: Pipeline: Could not create texture view" << std::endl;
    return false;
  }

  wgpu::CommandEncoderDescriptor encoderDesc = {{.label = "lab default command encoder"}};
  wgpu::CommandEncoder encoder = self->webgpu.device.createCommandEncoder(encoderDesc);

  self->render_config.renderPassColorAttachment.view = targetView;
  self->render_config.renderPassDesc.colorAttachmentCount = 1;
  self->render_config.renderPassDesc.colorAttachments =
      &self->render_config.renderPassColorAttachment;

  wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(self->render_config.renderPassDesc);
  renderPass.setPipeline(self->wgpu_pipeline);

  for (uint32_t i = 0; i < self->vb_configs.size(); ++i) {
    renderPass.setVertexBuffer(i, self->vb_configs[i].buffer, self->vb_configs[i].offset,
                               self->vb_configs[i].buffer.getSize());
  }

  renderPass.setBindGroup(0, self->bindGroup, 0, nullptr);

  renderPass.draw(draw_params.vertexCount, draw_params.instanceCount, draw_params.firstVertex,
                  draw_params.firstInstance);

  renderPass.end();
  renderPass.release();

  wgpu::CommandBufferDescriptor cmdBufferDescriptor = {{.label = "My command buffer"}};
  wgpu::CommandBuffer commands = encoder.finish(cmdBufferDescriptor);
  encoder.release();

  self->webgpu.queue.submit(commands);
  commands.release();

  targetView.release();
  surface.present();
  self->webgpu.device.tick();

  return true;
};

wgpu::TextureView Pipeline::get_target_view(wgpu::Surface surface) {
  wgpu::SurfaceTexture surfaceTexture;
  surface.getCurrentTexture(&surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    std::cerr << "Error: Pipeline: Could not get current render texture" << std::endl;
    return nullptr;
  }
  WGPUTextureViewDescriptor viewDescriptor{
      .label = "lab default texture view",
      .format = wgpuTextureGetFormat(surfaceTexture.texture),
      .dimension = WGPUTextureViewDimension_2D,
      .baseMipLevel = 0,
      .mipLevelCount = 1,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .aspect = WGPUTextureAspect_All,
  };
  return wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);
}

} // namespace lab
