#ifndef WGPU_LAB_PIPELINE_H
#define WGPU_LAB_PIPELINE_H

#include "lab_shader.h"
#include "lab_surface.h"
#include "lab_webgpu.h"

namespace lab {

struct PipelineInitConfig {
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

  wgpu::ColorTargetState colorTarget = {{
      .writeMask = wgpu::ColorWriteMask::All,
  }};

  wgpu::FragmentState fragmentState = {{
      .entryPoint = "fs_main",
  }};

  wgpu::VertexState vertexState = {{
      .entryPoint = "vs_main",
  }};

  wgpu::PrimitiveState primitiveState = {{
      .topology = wgpu::PrimitiveTopology::TriangleList,
      .stripIndexFormat = wgpu::IndexFormat::Undefined,
      .frontFace = wgpu::FrontFace::CCW,
      .cullMode = wgpu::CullMode::None,
  }};

  wgpu::MultisampleState multisampleState = {{
      .count = 1,
      .mask = ~0u,
  }};

  std::string label;
};

struct PipelineRenderConfig {
  // todo
  //

  wgpu::RenderPassColorAttachment renderPassColorAttachment = {{
      .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
      .loadOp = wgpu::LoadOp::Clear,
      .storeOp = wgpu::StoreOp::Store,
      .clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0},
  }};

  wgpu::RenderPassDescriptor renderPassDesc = {{
      .label = "My render pass",
      .colorAttachmentCount = 1,
  }};
};

struct Pipeline {
  using RenderFunction = std::function<bool(Pipeline& self, Surface&)>;
  Shader& shader;
  Webgpu& webgpu;

  PipelineInitConfig config;
  PipelineRenderConfig render_config;

  wgpu::RenderPipeline wgpu_pipeline;

  Pipeline(Shader&, Webgpu&, bool create_now = false);

  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  void create();
  wgpu::RenderPipeline create(wgpu::ShaderModule);

  bool render_frame(Surface& surface);
  RenderFunction render_func = [](Pipeline& self, Surface& surface) {
    wgpu::SurfaceTexture surfaceTexture;
    surface.wgpu_surface.getCurrentTexture(&surfaceTexture);
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
      std::cerr << "Error: Pipeline: Could not get current texture" << std::endl;
      return false;
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
    wgpu::TextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);
    if (!targetView) {
      std::cerr << "Error: Pipeline: Could not create texture view" << std::endl;
      return false;
    }

    wgpu::CommandEncoderDescriptor encoderDesc = {{.label = "lab default command encoder"}};
    wgpu::CommandEncoder encoder = self.webgpu.device.createCommandEncoder(encoderDesc);

    self.render_config.renderPassColorAttachment.view = targetView;
    self.render_config.renderPassDesc.colorAttachmentCount = 1;
    self.render_config.renderPassDesc.colorAttachments =
        &self.render_config.renderPassColorAttachment;

    wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(self.render_config.renderPassDesc);
    renderPass.setPipeline(self.wgpu_pipeline);

    renderPass.draw(3, 1, 0, 0);

    // todo: user_draw(renderPass);

    renderPass.end();
    renderPass.release();

    wgpu::CommandBufferDescriptor cmdBufferDescriptor = {{.label = "My command buffer"}};
    wgpu::CommandBuffer commands = encoder.finish(cmdBufferDescriptor);
    encoder.release();

    self.webgpu.queue.submit(commands);
    commands.release();

    targetView.release();
    surface.wgpu_surface.present();
    self.webgpu.device.tick();

    return true;
  };

  ~Pipeline();
};

} // namespace lab

#endif // WGPU_LAB_PIPELINE_H
