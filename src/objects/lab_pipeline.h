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

  // Initializes a pipeline
  // 1. creates shader module on gpu
  // 2. bundles together default + user configs
  // 3. creates render pipeline on gpu
  // 4. releases shader module
  // 5. render pipeline is released later in destructor
  void finalize() {
    wgpu::ShaderModule shaderModule = shader.transfer(webgpu.device);
    finalize_config(shaderModule);

    std::cout << "Info: WGPU: Create: " << label << std::endl;
    assert(wgpu_pipeline == nullptr);
    wgpu_pipeline = transfer();

    shaderModule.release();
  }

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

  static wgpu::TextureView get_target_view(wgpu::Surface surface);
  static bool default_render(PipelineHandle self, wgpu::Surface surface,
                             const DrawCallParams& draw_params);

  ~Pipeline();

  // -----------------------------------------------------------------------------------------------
  // Configurable structures and functions with default values -------------------------------------

  // All config fields that have not been assigned a default value other than null
  // are non-configurable and will be overwritten in `finalize_config()` to guarantee consistency
  pipeline_defaults::InitConfig config{};

  // All config fields that have not been assigned a default value other than null
  // are non-configurable and will be overwritten in `finalize_config()`
  pipeline_defaults::RenderConfig render_config{};

  // Can be reassigned with custom render function
  RenderFunction render_func = default_render;
  // -----------------------------------------------------------------------------------------------

  wgpu::RenderPipeline wgpu_pipeline;

  // Default label: `Pipeline(shader.label, webgpu.label)`
  // if required, can be customized by assigning before `init()`
  std::string label;

  Webgpu& webgpu;
  Shader& shader;

  // Work in progress ------------------------------
  struct VertexBufferConfig {
    wgpu::Buffer buffer;
    wgpu::VertexStepMode mode;
    uint64_t offset;
    std::vector<wgpu::VertexAttribute> vertexAttributes;
  };
  std::vector<VertexBufferConfig> vertex_buffer_configs;
  std::vector<wgpu::VertexBufferLayout> vb_layouts;

  void add_vertex_buffer(wgpu::Buffer wgpu_buffer,
                         wgpu::VertexStepMode mode = wgpu::VertexStepMode::Vertex,
                         uint64_t offset = 0u) {
    assert(wgpu_buffer.getUsage() & wgpu::BufferUsage::Vertex);
    vertex_buffer_configs.push_back({wgpu_buffer, mode, offset});
  }

  void add_vertex_attribute(wgpu::VertexFormat format, uint32_t shader_location,
                            uint64_t offset = 0, uint32_t buffer_index = ~0u) {
    vertex_buffer_configs.at(buffer_index == ~0 ? vertex_buffer_configs.size() - 1 : buffer_index)
        .vertexAttributes.push_back({{
            .format = format,
            .offset = offset,
            .shaderLocation = shader_location,
        }});
  }

  uint64_t get_total_stride(const std::vector<wgpu::VertexAttribute>& vertexAttributes) {
    uint64_t totalStride = 0;
    for (const auto& va : vertexAttributes) {
      switch (va.format) {
      case wgpu::VertexFormat::Float32x2:
        totalStride += 2 * sizeof(float);
        break;
      case wgpu::VertexFormat::Float32:
        totalStride += 1 * sizeof(float);
        break;
      }
      // TODO: add cases for all vertex formats
    }
    return totalStride;
  }
};

} // namespace lab

#endif // WGPU_LAB_PIPELINE_H
