#include "lab_pipeline.h"

#include <iostream>

namespace lab {

void Pipeline::render_frame() {
  wgpu::SurfaceTexture surfaceTexture;
  surface.wgpu_surface.getCurrentTexture(&surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    std::cerr << "Error: WGPU: could not get current texture" << std::endl;
    return;
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
    std::cerr << "Error: WGPU: Could not create texture view" << std::endl;
    return;
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
}

Pipeline::Pipeline(Webgpu& wg, Shader& sh, Surface& sf) : webgpu{wg}, shader{sh}, surface{sf} {
  std::cout << "Info: WGPU: Create Pipeline" << std::endl;

  shader.transfer(webgpu.device);

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
      .module = shader.wgpu_shadermodule,
      .entryPoint = "fs_main",
      .targetCount = 1,
      .targets = &colorTarget,
  }};

  wgpu::RenderPipelineDescriptor pipelineDesc = {{
      .label = "My render pipeline",
      .vertex = {.module = shader.wgpu_shadermodule, .entryPoint = "vs_main"},
      .primitive = {.topology = wgpu::PrimitiveTopology::TriangleList,
                    .stripIndexFormat = wgpu::IndexFormat::Undefined,
                    .frontFace = wgpu::FrontFace::CCW,
                    .cullMode = wgpu::CullMode::None},
      .multisample = {.count = 1, .mask = ~0u},
      .fragment = &fragmentState,
  }};

  wgpu_pipeline = webgpu.device.createRenderPipeline(pipelineDesc);
}

Pipeline::~Pipeline() {
  if (wgpu_pipeline) {
    std::cout << "Info: Release Pipeline" << std::endl;
    wgpu_pipeline.release();
    wgpu_pipeline = nullptr;
  }
}

} // namespace lab
