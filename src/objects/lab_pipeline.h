#ifndef WGPU_LAB_PIPELINE_H
#define WGPU_LAB_PIPELINE_H

#include <objects/lab_shader.h>
#include <objects/lab_surface.h>
#include <objects/lab_webgpu.h>

#include <webgpu/webgpu.hpp>

namespace lab {

// Is responsible to render frames onto surfaces
struct Pipeline {
  // - param: `configurable = false` makes pipeline init-stage non-configurable
  //   and causes automatic call to `init()` inside constructor.
  // If configuration is desired:
  // ```cpp
  // lab::Pipeline pip(my_shader, my_webgpu, false);
  //
  // // explore and modify `wgpu::` config objects:
  // // Example:
  // pip.config.primitiveState.topology = wgpu::PrimitiveTopology::LineStrip;
  //
  // pip.init(); // now ready to call init
  // ```
  // render_frame stage can always be configured by changing `render_config`
  // and, if needed, by `set_custom_renderfunc`
  Pipeline(Shader& sh, Webgpu& wg, bool configurable = false) : shader{sh}, webgpu{wg} {
    if (!configurable) init();
  }
  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  struct DrawCallParams {
    int vertexCount, instanceCount;
    int firstVertex, firstInstance;
  };
  using RenderFunction = std::function<bool(Pipeline& self, wgpu::Surface, DrawCallParams)>;

  bool render_frame(Surface& surface, const DrawCallParams& draw_params) {
    return user_render(*this, surface.wgpu_surface, draw_params);
  }
  void set_custom_renderfunc(RenderFunction func) { user_render = func; }

  void finalize_config(wgpu::ShaderModule);

  [[nodiscard]] wgpu::RenderPipeline transfer() const;

  void init() {
    wgpu::ShaderModule shaderModule = shader.transfer(webgpu.device);
    finalize_config(shaderModule);

    std::cout << "Info: WGPU: Create: " << config.label << std::endl;
    assert(wgpu_pipeline == nullptr);
    wgpu_pipeline = transfer();

    shaderModule.release();
  }

  ~Pipeline();

  // ---------------------------------------------------------------------------
  // Configurable functions and structures with default values

  RenderFunction user_render = default_render;

  // All config fields that have not been assigned a default value other than null
  // are non-configurable and will be overwritten in `finalize_config()`
  struct InitConfig {
    wgpu::BlendState blendState = {{
        .color =
            {
                .operation = wgpu::BlendOperation::Add,
                .srcFactor = wgpu::BlendFactor::SrcAlpha,
                .dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha,
            },
        .alpha =
            {
                .operation = wgpu::BlendOperation::Add,
                .srcFactor = wgpu::BlendFactor::Zero,
                .dstFactor = wgpu::BlendFactor::One,
            },
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
  } config;

  // All config fields that have not been assigned a default value other than null
  // are non-configurable and will be overwritten in `finalize_config()`
  struct RenderConfig {
    wgpu::RenderPassColorAttachment renderPassColorAttachment = {{
        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
        .loadOp = wgpu::LoadOp::Clear,
        .storeOp = wgpu::StoreOp::Store,
        .clearValue = WGPUColor{0.08, 0.08, 0.085, 1.0},
    }};

    wgpu::RenderPassDescriptor renderPassDesc = {{
        .label = "Default Render Pass",
        .colorAttachmentCount = 1,
    }};
  } render_config;

  static bool default_render(Pipeline& self, wgpu::Surface surface,
                             const DrawCallParams& draw_params) {
    wgpu::TextureView targetView = get_target_view(surface);
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

  static wgpu::TextureView get_target_view(wgpu::Surface surface) {
    wgpu::SurfaceTexture surfaceTexture;
    surface.getCurrentTexture(&surfaceTexture);
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
      std::cerr << "Error: Pipeline: Could not get current texture" << std::endl;
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

  wgpu::RenderPipeline wgpu_pipeline;
  Webgpu& webgpu;
  Shader& shader;
};

} // namespace lab

#endif // WGPU_LAB_PIPELINE_H
