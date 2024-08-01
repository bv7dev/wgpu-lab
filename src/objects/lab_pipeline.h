#ifndef WGPU_LAB_PIPELINE_H
#define WGPU_LAB_PIPELINE_H

#include <extra/lab_objects.h>
#include <extra/lab_pipeline_defaults.h>

#include <objects/lab_buffer.h>
#include <objects/lab_shader.h>
#include <objects/lab_surface.h>
#include <objects/lab_webgpu.h>

#include <webgpu/webgpu.hpp>

namespace lab {

// Is responsible to manage the process of rendering frames onto surfaces
struct Pipeline {
  // Creates a new pipeline object
  // ```cpp
  // lab::Pipeline pip(my_shader, my_webgpu, false);
  //
  // // explore and modify config objects.
  // pip.config.primitiveState.topology = wgpu::PrimitiveTopology::LineStrip;
  //
  // pip.init(); // now ready to call init
  // ```
  // render_frame stage can always be configured by changing `render_config`
  // and, if needed, by `set_custom_renderfunc(func)`
  Pipeline(Shader& sh, Webgpu& wg) : shader{sh}, webgpu{wg} {}

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

  // Warning! User is responsible to `.release()` the returned render pipeline
  // - be careful not to leak resources if you need to use it directly
  // - by default, it's automatically managed, see: `Pipeline::init()`
  [[nodiscard]] wgpu::RenderPipeline transfer() const;

  // Bundles together user_config and default_config
  void finalize_config(wgpu::ShaderModule);

  // Initializes a pipeline
  // 1. creates shader module on gpu
  // 2. bundles together default + user configs
  // 3. creates render pipeline on gpu
  // 4. releases shader module
  // 5. render pipeline is released later in destructor
  void init(wgpu::Buffer wgpu_buffer = nullptr) {
    vertexBuffer = wgpu_buffer;

    wgpu::ShaderModule shaderModule = shader.transfer(webgpu.device);
    finalize_config(shaderModule);

    std::cout << "Info: WGPU: Create: " << label << std::endl;
    assert(wgpu_pipeline == nullptr);
    wgpu_pipeline = transfer();

    shaderModule.release();
  }

  static wgpu::TextureView get_target_view(wgpu::Surface surface);
  static bool default_render(PipelineHandle self, wgpu::Surface surface,
                             const DrawCallParams& draw_params);

  ~Pipeline();

  wgpu::RenderPipeline wgpu_pipeline;

  // internal handle to which vertex buffer to use for rendering
  wgpu::Buffer vertexBuffer;

  // Default label: `Pipeline(shader.label, webgpu.label)`
  // if required, can be customized by assigning before `init()`
  std::string label;

  Webgpu& webgpu;
  Shader& shader;

  // -----------------------------------------------------------------------------------------------
  // Configurable functions and structures with default values -------------------------------------

  // All config fields that have not been assigned a default value other than null
  // are non-configurable and will be overwritten in `finalize_config()` to guarantee consistency
  pipeline_defaults::InitConfig config{};

  // All config fields that have not been assigned a default value other than null
  // are non-configurable and will be overwritten in `finalize_config()`
  pipeline_defaults::RenderConfig render_config{};

  // Can be reassigned with custom render function
  RenderFunction render_func = default_render;
};

} // namespace lab

#endif // WGPU_LAB_PIPELINE_H
