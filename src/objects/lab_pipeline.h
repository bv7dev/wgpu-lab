#ifndef WGPU_LAB_PIPELINE_H
#define WGPU_LAB_PIPELINE_H

#include <extra/lab_objects.h>
#include <extra/lab_pipeline_defaults.h>

#include <objects/lab_shader.h>
#include <objects/lab_surface.h>
#include <objects/lab_webgpu.h>

#include <webgpu/webgpu.hpp>

namespace lab {

// Is responsible to manage the process of rendering frames onto surfaces
struct Pipeline {
  // Creates a new pipeline object
  // - param: `configurable = false` makes pipeline init-stage non-configurable
  //   and causes automatic call to `init()` inside constructor.
  //
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
    uint32_t vertexCount, instanceCount;
    uint32_t firstVertex, firstInstance;
  };
  using RenderFunction = std::function<bool(PipelineHandle self, wgpu::Surface, DrawCallParams)>;

  // Render onto surface
  // - param: `draw_params` is `{vertexCount, instanceCount, firstVertex, firstInstance}`
  bool render_frame(Surface& surface, const DrawCallParams& draw_params = {3, 1}) {
    return render_func(this, surface.wgpu_surface, draw_params);
  }

  // Bundles together user_config and default_config
  void finalize_config(wgpu::ShaderModule);

  // Warning! User is responsible to `.release()` the returned render pipeline
  // - be careful not to leak resources if you need to use it directly
  // - by default, it's automatically managed, see: `Pipeline::init()`
  [[nodiscard]] wgpu::RenderPipeline transfer() const;

  // Initializes a pipeline
  // 1. creates shader module on gpu
  // 2. bundles together default + user configs
  // 3. creates render pipeline on gpu
  // 4. releases shader module
  // 5. render pipeline is released later in destructor
  void init() {
    wgpu::ShaderModule shaderModule = shader.transfer(webgpu.device);
    finalize_config(shaderModule);

    std::cout << "Info: WGPU: Create: " << label << std::endl;
    assert(wgpu_pipeline == nullptr);
    wgpu_pipeline = transfer();

    shaderModule.release();
  }

  ~Pipeline();

  wgpu::RenderPipeline wgpu_pipeline;

  Webgpu& webgpu;
  Shader& shader;

  // -----------------------------------------------------------------------------------------------
  // Configurable functions and structures with default values -------------------------------------

  // Default label: `Pipeline(shader.label, webgpu.label)`
  // if required, can be customized by assigning before `init()`
  std::string label;

  // All config fields that have not been assigned a default value other than null
  // are non-configurable and will be overwritten in `finalize_config()` to guarantee consistency
  PipelineDefaults::InitConfig config{};

  // All config fields that have not been assigned a default value other than null
  // are non-configurable and will be overwritten in `finalize_config()`
  PipelineDefaults::RenderConfig render_config{};

  // Can be reassigned with custom render function
  RenderFunction render_func = default_render;

  wgpu::Buffer vertex_buffer;

  // -----------------------------------------------------------------------------------------------
  // default RenderFunction implementation details to base custom render function on ---------------

  static bool default_render(PipelineHandle self, wgpu::Surface surface,
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

    wgpu::RenderPassEncoder renderPass =
        encoder.beginRenderPass(self->render_config.renderPassDesc);
    renderPass.setPipeline(self->wgpu_pipeline);

    if (self->vertex_buffer) {
      renderPass.setVertexBuffer(0, self->vertex_buffer, 0, self->vertex_buffer.getSize());
    }

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

  static wgpu::TextureView get_target_view(wgpu::Surface surface) {
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
};

} // namespace lab

#endif // WGPU_LAB_PIPELINE_H
