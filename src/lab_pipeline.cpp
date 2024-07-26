#include "lab_pipeline.h"

#include <iostream>

namespace lab {

Pipeline::Pipeline(Shader& sh, Webgpu& wg) : shader{sh}, webgpu{wg} {
  std::cout << "Info: WGPU: Create Pipeline" << std::endl;
  wgpu::ShaderModule shaderModule = shader.transfer(webgpu.device);

  wgpu::BlendComponent blendColor = {{
      .operation = wgpu::BlendOperation::Add,
      .srcFactor = wgpu::BlendFactor::SrcAlpha,
      .dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha,
  }};

  wgpu::BlendComponent blendAlpha = {{
      .operation = wgpu::BlendOperation::Add,
      .srcFactor = wgpu::BlendFactor::Zero,
      .dstFactor = wgpu::BlendFactor::One,
  }};

  wgpu::BlendState blendState = {{
      .color = blendColor,
      .alpha = blendAlpha,
  }};

  wgpu::ColorTargetState colorTarget = {{
      .format = webgpu.capabilities.formats[0],
      .blend = &blendState,
      .writeMask = wgpu::ColorWriteMask::All,
  }};

  wgpu::FragmentState fragmentState = {{
      .module = shaderModule,
      .entryPoint = "fs_main",
      .targetCount = 1,
      .targets = &colorTarget,
  }};

  wgpu::RenderPipelineDescriptor pipelineDesc = {{
      .label = "My render pipeline",
      .vertex = {.module = shaderModule, .entryPoint = "vs_main"},
      .primitive = {.topology = wgpu::PrimitiveTopology::TriangleList,
                    .stripIndexFormat = wgpu::IndexFormat::Undefined,
                    .frontFace = wgpu::FrontFace::CCW,
                    .cullMode = wgpu::CullMode::None},
      .multisample = {.count = 1, .mask = ~0u},
      .fragment = &fragmentState,
  }};

  wgpu_pipeline = webgpu.device.createRenderPipeline(pipelineDesc);
  shaderModule.release();
}

bool Pipeline::render_frame(Surface& surface) const {
  wgpu::SurfaceTexture surfaceTexture;
  surface.wgpu_surface.getCurrentTexture(&surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    std::cerr << "Error: Pipeline::render_frame: Could not get current texture" << std::endl;
    return false;
  }
  WGPUTextureViewDescriptor viewDescriptor{
      .label = "My texture view",
      .format = wgpuTextureGetFormat(surfaceTexture.texture),
      .dimension = WGPUTextureViewDimension_2D,
      .baseMipLevel = 0,
      .mipLevelCount = 1,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .aspect = WGPUTextureAspect_All,
  };
  wgpu::TextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);
  if (!targetView) {
    std::cerr << "Error: Pipeline::render_frame: Could not create texture view" << std::endl;
    return false;
  }

  wgpu::CommandEncoderDescriptor encoderDesc = {{.label = "My command encoder"}};
  wgpu::CommandEncoder encoder = webgpu.device.createCommandEncoder(encoderDesc);

  wgpu::RenderPassColorAttachment renderPassColorAttachment = {{
      .view = targetView,
      .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
      .loadOp = wgpu::LoadOp::Clear,
      .storeOp = wgpu::StoreOp::Store,
      .clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0},
  }};

  wgpu::RenderPassDescriptor renderPassDesc = {{
      .label = "My render pass",
      .colorAttachmentCount = 1,
      .colorAttachments = &renderPassColorAttachment,
  }};

  wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
  renderPass.setPipeline(wgpu_pipeline);

  renderPass.draw(3, 1, 0, 0);

  renderPass.end();
  renderPass.release();

  wgpu::CommandBufferDescriptor cmdBufferDescriptor = {{.label = "My command buffer"}};
  wgpu::CommandBuffer commands = encoder.finish(cmdBufferDescriptor);
  encoder.release();

  webgpu.queue.submit(commands);
  commands.release();

  targetView.release();
  surface.wgpu_surface.present();
  webgpu.device.tick();

  return true;
}

Pipeline::~Pipeline() {
  if (wgpu_pipeline) {
    std::cout << "Info: Release Pipeline" << std::endl;
    wgpu_pipeline.release();
    wgpu_pipeline = nullptr;
  }
}

} // namespace lab
