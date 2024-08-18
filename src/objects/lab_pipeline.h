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

uint64_t vertex_format_size(wgpu::VertexFormat);

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
    assert(wgpu_pipeline == nullptr);

    wgpu::ShaderModule shaderModule = shader.transfer(webgpu.device);
    finalize_config(shaderModule);

    std::cout << "Info: WGPU: Create: " << label << std::endl;
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

  // static wgpu::TextureView get_current_render_texture_view(wgpu::Surface surface);
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
  std::vector<VertexBufferConfig> vb_configs;
  std::vector<wgpu::VertexBufferLayout> vb_layouts;

  void add_vertex_buffer(wgpu::Buffer wgpu_buffer,
                         wgpu::VertexStepMode mode = wgpu::VertexStepMode::Vertex,
                         uint64_t offset = 0) {
    assert(wgpu_buffer.getUsage() & wgpu::BufferUsage::Vertex);
    vb_configs.push_back({wgpu_buffer, mode, offset});
  }

  template<typename T>
  void add_vertex_buffer(const Buffer<T>& buffer,
                         wgpu::VertexStepMode mode = wgpu::VertexStepMode::Vertex,
                         uint64_t offset = 0) {
    add_vertex_buffer(buffer.wgpu_buffer, mode, offset);
  }

  void add_vertex_attribute(wgpu::VertexFormat format, uint32_t shader_location,
                            uint64_t offset = ~0, uint64_t buffer_index = ~0) {
    uint64_t bi = buffer_index == ~0 ? vb_configs.size() - 1 : buffer_index;
    if (vb_configs.at(bi).vertexAttributes.size() > 0) {
      offset = offset == ~0 ? vertex_format_size(vb_configs.at(bi).vertexAttributes.back().format)
                            : offset;
    } else {
      offset = 0;
    }
    vb_configs.at(bi).vertexAttributes.push_back({{
        .format = format,
        .offset = offset,
        .shaderLocation = shader_location,
    }});
  }

  // WIP 2 uniform buffer layouts ==========================================================
  template<typename T>
  void add_uniform_buffer(const Buffer<T>& uniform_buffer, uint32_t binding_index,
                          wgpu::ShaderStageFlags visibility) {
    add_bind_group_entry(uniform_buffer.wgpu_buffer, binding_index, sizeof(T));
    add_bind_group_layout_entry(0, visibility, wgpu::BufferBindingType::Uniform, sizeof(T));
    finalize_bind_group();
  }

  std::vector<wgpu::BindGroupEntry> bindGroupEntries{};
  void add_bind_group_entry(wgpu::Buffer wgpu_buffer, uint32_t binding_index, uint64_t size,
                            uint64_t offset = 0) {
    wgpu::BindGroupEntry bindGroupEntry = wgpu::Default;
    bindGroupEntry.buffer = wgpu_buffer;
    bindGroupEntry.binding = binding_index;
    bindGroupEntry.size = size;
    bindGroupEntry.offset = offset;
    bindGroupEntries.push_back(bindGroupEntry);
  }

  std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntries{};
  void add_bind_group_layout_entry(uint32_t binding, WGPUShaderStageFlags visibility,
                                   WGPUBufferBindingType buffer_type, uint64_t min_binding_size) {
    wgpu::BindGroupLayoutEntry bindGroupLayoutEntry = wgpu::Default;
    bindGroupLayoutEntry.binding = binding;
    bindGroupLayoutEntry.visibility = visibility;
    bindGroupLayoutEntry.buffer.type = buffer_type;
    bindGroupLayoutEntry.buffer.minBindingSize = min_binding_size;
    bindGroupLayoutEntries.push_back(bindGroupLayoutEntry);
  }

  // todo: WIP textures -------------------
  void add_bind_group_layout_texture_entry(uint32_t binding, WGPUShaderStageFlags visibility) {
    wgpu::BindGroupLayoutEntry bindGroupLayoutEntry = wgpu::Default;
    bindGroupLayoutEntry.binding = binding;
    bindGroupLayoutEntry.visibility = visibility;

    bindGroupLayoutEntry.texture.multisampled = false;
    bindGroupLayoutEntry.texture.sampleType = wgpu::TextureSampleType::Float;
    bindGroupLayoutEntry.texture.viewDimension = wgpu::TextureViewDimension::_2D;

    bindGroupLayoutEntries.push_back(bindGroupLayoutEntry);
  }

  void add_bind_group_texture_entry(wgpu::TextureView texture_view, uint32_t binding_index) {
    wgpu::BindGroupEntry bindGroupEntry = wgpu::Default;
    bindGroupEntry.textureView = texture_view;
    bindGroupEntry.binding = binding_index;
    bindGroupEntries.push_back(bindGroupEntry);
  }
  // END todo: WIP textures -------------------

  std::vector<WGPUBindGroupLayout> bindGroupLayouts{};
  std::vector<wgpu::BindGroup> bindGroups{};
  void finalize_bind_group(const char* group_label = "Default Bind Group") {
    std::string groupLabel{label};
    groupLabel.append(" - ");
    groupLabel.append(group_label);

    std::string layoutLabel{groupLabel};
    layoutLabel.append(" Layout");

    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc = wgpu::Default;
    bindGroupLayoutDesc.entryCount = bindGroupLayoutEntries.size();
    bindGroupLayoutDesc.entries = bindGroupLayoutEntries.data();
    bindGroupLayoutDesc.label = layoutLabel.c_str();

    bindGroupLayouts.push_back(webgpu.device.createBindGroupLayout(bindGroupLayoutDesc));

    wgpu::BindGroupDescriptor bindGroupDesc = wgpu::Default;
    bindGroupDesc.layout = bindGroupLayouts.back();
    bindGroupDesc.entryCount = bindGroupEntries.size();
    bindGroupDesc.entries = bindGroupEntries.data();
    bindGroupDesc.label = groupLabel.c_str();

    bindGroups.push_back(webgpu.device.createBindGroup(bindGroupDesc));

    bindGroupEntries.clear();
    bindGroupLayoutEntries.clear();
  }
};

} // namespace lab

#endif // WGPU_LAB_PIPELINE_H
