#ifndef WGPU_LAB_PIPELINE_H
#define WGPU_LAB_PIPELINE_H

#include <extra/lab_objects.h>
#include <extra/lab_pipeline_defaults.h>

#include <objects/lab_buffer.h>
#include <objects/lab_shader.h>
#include <objects/lab_surface.h>
#include <objects/lab_texture.h>
#include <objects/lab_webgpu.h>

#include <dawn/webgpu_cpp.h>

namespace lab {

// returns size in bytes of given wgpu::VertexFormat
constexpr uint64_t vertex_format_size(const wgpu::VertexFormat& format);

// returns the total stride of a given vector of wgpu::VertexAttribute objects
uint64_t vertex_attributes_stride(const std::vector<wgpu::VertexAttribute>& vertexAttributes);

// todo: remove ?
wgpu::TextureView get_current_render_texture_view(wgpu::Surface surface);

// Is responsible to manage the process of rendering frames onto surfaces
struct Pipeline {
  // Creates a new pipeline object
  // ```cpp
  // lab::Pipeline pip(my_shader, my_webgpu);
  //
  // // explore and modify config objects.
  // pip.config.primitiveState.topology = wgpu::PrimitiveTopology::LineStrip;
  //
  // pip.finalize(); // now ready to use for rendering
  // ```
  // render_frame stage can always be configured by changing `render_config`
  // and, if needed, by `set_custom_renderfunc(func)`
  //
  // set param `final = true` if there is no need to configure anything -
  // this makes the pipeline ready to use.
  Pipeline(Shader& sh, Webgpu& wg, bool final = false) : shader{sh}, webgpu{wg} {
    if (final) finalize();
  }

  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  // Initializes a pipeline
  // 1. creates shader module on gpu
  // 2. combines default + user configs
  // 3. creates texture and buffer bindings
  // 4. creates render pipeline on gpu
  // 5. releases shader module
  void finalize() {
    reset();
    wgpu::ShaderModule shaderModule = shader.transfer(webgpu.device);
    finalize_config(shaderModule);
    finalize_bind_group();
    wgpu_pipeline = transfer();
  }

  struct DrawCallParams {
    uint32_t vertexCount, instanceCount;
    uint32_t firstVertex, firstInstance;
  };
  using RenderFunction = std::function<bool(PipelineHandle self, wgpu::Surface, const DrawCallParams&)>;

  // Render onto surface (DrawCallParams{vertexCount, instanceCount, firstVertex, firstInstance})
  bool render_frame(const Surface& surface, const DrawCallParams& params) {
    return render_func(this, surface.wgpu_surface, params);
  }

  // Render `vertex_count` vertices onto a surface
  bool render_frame(const Surface& surface, std::integral auto vertex_count) {
    return render_func(this, surface.wgpu_surface, {static_cast<uint32_t>(vertex_count), 1, 0, 0});
  }

  // Render `vertex_count` vertices onto a surface `instance_count` times
  bool render_frame(const Surface& surface, std::integral auto vertex_count, std::integral auto instance_count) {
    return render_func(this, surface.wgpu_surface,
                       {static_cast<uint32_t>(vertex_count), static_cast<uint32_t>(instance_count), 0, 0});
  }

  // Warning! User is responsible to `.release()` the returned render pipeline
  // - be careful not to leak resources if you need to use it directly
  // - by default, it's automatically managed, see: `Pipeline::init()`
  [[nodiscard]] wgpu::RenderPipeline transfer() const;

  // Bundles together user_config and default_config
  void finalize_config(wgpu::ShaderModule);

  static bool default_render(PipelineHandle self, wgpu::Surface surface, const DrawCallParams& draw_params);

  void reset();
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

  void add_vertex_buffer(wgpu::Buffer wgpu_buffer, wgpu::VertexStepMode mode = wgpu::VertexStepMode::Vertex,
                         uint64_t offset = 0) {
    assert(wgpu_buffer.GetUsage() & wgpu::BufferUsage::Vertex);
    vb_configs.push_back({wgpu_buffer, mode, offset});
  }

  template<typename T>
  void add_vertex_buffer(const Buffer<T>& buffer, wgpu::VertexStepMode mode = wgpu::VertexStepMode::Vertex,
                         uint64_t offset = 0) {
    add_vertex_buffer(buffer.wgpu_buffer, mode, offset);
  }

  void add_vertex_attrib(wgpu::VertexFormat format, uint32_t shader_location, uint64_t offset = ~0,
                         uint64_t buffer_index = ~0) {
    uint64_t bi = buffer_index == ~0 ? vb_configs.size() - 1 : buffer_index;
    offset = offset == ~0 ? vertex_attributes_stride(vb_configs.at(bi).vertexAttributes) : offset;
    vb_configs.at(bi).vertexAttributes.push_back({
        .format = format,
        .offset = offset,
        .shaderLocation = shader_location,
    });
  }

  struct IndexBufferConfig {
    wgpu::Buffer buffer;
    wgpu::IndexFormat format;
    uint64_t offset;
  };

  std::vector<IndexBufferConfig> ib_configs;

  void add_index_buffer(wgpu::Buffer buffer, wgpu::IndexFormat format, uint64_t offset = 0) {
    ib_configs.push_back({buffer, format, offset});
  }

  template<typename T>
  void add_index_buffer(const Buffer<T>& buffer, wgpu::IndexFormat format, uint64_t offset = 0) {
    add_index_buffer(buffer.wgpu_buffer, format, offset);
  }

  template<typename T>
  void add_uniform_buffer(const Buffer<T>& uniform_buffer, uint32_t binding_index, wgpu::ShaderStage visibility) {
    add_bind_group_layout_buffer_entry(0, visibility, wgpu::BufferBindingType::Uniform, sizeof(T));
    add_bind_group_buffer_entry(uniform_buffer.wgpu_buffer, binding_index, sizeof(T));
  }

  void add_texture(const Texture& texture, uint32_t binding,
                   wgpu::ShaderStage visibility = wgpu::ShaderStage::Fragment) {
    add_bind_group_layout_texture_entry(binding, visibility);
    add_bind_group_texture_entry(texture.create_view(), binding);
  }

  std::vector<wgpu::BindGroupLayoutEntry> bindGroupLayoutEntries{};
  std::vector<wgpu::BindGroupEntry> bindGroupEntries{};

  void add_bind_group_layout_buffer_entry(uint32_t binding, wgpu::ShaderStage visibility,
                                          wgpu::BufferBindingType buffer_type, uint64_t min_binding_size) {
    wgpu::BindGroupLayoutEntry bindGroupLayoutEntry;
    bindGroupLayoutEntry.binding = binding;
    bindGroupLayoutEntry.visibility = visibility;

    bindGroupLayoutEntry.buffer.type = buffer_type;
    bindGroupLayoutEntry.buffer.minBindingSize = min_binding_size;

    bindGroupLayoutEntries.push_back(bindGroupLayoutEntry);
  }

  void add_bind_group_buffer_entry(wgpu::Buffer wgpu_buffer, uint32_t binding_index, uint64_t size,
                                   uint64_t offset = 0) {
    wgpu::BindGroupEntry bindGroupEntry;
    bindGroupEntry.buffer = wgpu_buffer;
    bindGroupEntry.binding = binding_index;

    bindGroupEntry.size = size;
    bindGroupEntry.offset = offset;

    bindGroupEntries.push_back(bindGroupEntry);
  }

  void add_bind_group_layout_texture_entry(uint32_t binding, wgpu::ShaderStage visibility,
                                           wgpu::TextureSampleType sampleType = wgpu::TextureSampleType::Float,
                                           wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D,
                                           wgpu::Bool multisampled = false) {
    wgpu::BindGroupLayoutEntry bindGroupLayoutEntry;
    bindGroupLayoutEntry.binding = binding;
    bindGroupLayoutEntry.visibility = visibility;

    bindGroupLayoutEntry.texture.multisampled = multisampled;
    bindGroupLayoutEntry.texture.sampleType = sampleType;
    bindGroupLayoutEntry.texture.viewDimension = viewDimension;

    bindGroupLayoutEntries.push_back(bindGroupLayoutEntry);
  }

  void add_bind_group_texture_entry(wgpu::TextureView texture_view, uint32_t binding_index) {
    wgpu::BindGroupEntry bindGroupEntry;
    bindGroupEntry.textureView = texture_view;
    bindGroupEntry.binding = binding_index;

    bindGroupEntries.push_back(bindGroupEntry);
  }

  std::vector<wgpu::BindGroup> bindGroups{};
  std::vector<wgpu::BindGroupLayout> bindGroupLayouts{};

  void finalize_bind_group(const char* group_label = "lab default bind group") {
    std::string groupLabel{label};
    groupLabel.append(" - ");
    groupLabel.append(group_label);

    std::string layoutLabel{groupLabel};
    layoutLabel.append(" Layout");

    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.entryCount = bindGroupLayoutEntries.size();
    bindGroupLayoutDesc.entries = bindGroupLayoutEntries.data();
    bindGroupLayoutDesc.label = layoutLabel.c_str();

    bindGroupLayouts.push_back(webgpu.device.CreateBindGroupLayout(&bindGroupLayoutDesc));

    wgpu::BindGroupDescriptor bindGroupDesc;
    bindGroupDesc.layout = bindGroupLayouts.back();
    bindGroupDesc.entryCount = bindGroupEntries.size();
    bindGroupDesc.entries = bindGroupEntries.data();
    bindGroupDesc.label = groupLabel.c_str();

    bindGroups.push_back(webgpu.device.CreateBindGroup(&bindGroupDesc));

    bindGroupLayoutEntries.clear();
    bindGroupEntries.clear();
  }
};

} // namespace lab

#endif // WGPU_LAB_PIPELINE_H
