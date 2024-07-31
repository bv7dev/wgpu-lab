#ifndef WGPU_LAB_PIPELINE_H
#define WGPU_LAB_PIPELINE_H

#include "lab_shader.h"
#include "lab_surface.h"
#include "lab_webgpu.h"

#include <webgpu/webgpu.hpp>

namespace lab {

struct DrawCallParams {
  int vertexCount, instanceCount;
  int firstVertex, firstInstance;
};

struct Pipeline {
  using RenderFunction = std::function<bool(Pipeline& self, wgpu::Surface, DrawCallParams)>;

  Pipeline(Shader& sh, Webgpu& wg, bool init_now = false) : shader{sh}, webgpu{wg} {
    if (init_now) init();
  }
  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  bool render_frame(Surface& surface, const DrawCallParams& draw_params) {
    return user_render(*this, surface.wgpu_surface, draw_params);
  }
  void set_custom_renderfunc(RenderFunction func) { user_render = func; }

  void init() {
    wgpu::ShaderModule shaderModule = shader.transfer(webgpu.device);
    create(shaderModule);
    shaderModule.release();
  }
  wgpu::RenderPipeline create(wgpu::ShaderModule);

  ~Pipeline();

  // Configurable structures with default values
  struct InitConfig {
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

    std::string label;
  } config;

  struct RenderConfig {
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
  } render_config;

  static bool default_render(Pipeline& self, wgpu::Surface surface,
                             const DrawCallParams& draw_params) {
    wgpu::SurfaceTexture surfaceTexture;
    surface.getCurrentTexture(&surfaceTexture);
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

    renderPass.draw(draw_params.vertexCount, draw_params.instanceCount, draw_params.firstVertex,
                    draw_params.firstInstance);

    renderPass.end();
    renderPass.release();

    wgpu::CommandBufferDescriptor cmdBufferDescriptor = {{.label = "My command buffer"}};
    wgpu::CommandBuffer commands = encoder.finish(cmdBufferDescriptor);
    encoder.release();

    self.webgpu.queue.submit(commands);
    commands.release();

    targetView.release();
    surface.present();
    self.webgpu.device.tick();

    return true;
  };

  RenderFunction user_render = default_render;

  wgpu::RenderPipeline wgpu_pipeline;
  Webgpu& webgpu;
  Shader& shader;
};

} // namespace lab

#endif // WGPU_LAB_PIPELINE_H
