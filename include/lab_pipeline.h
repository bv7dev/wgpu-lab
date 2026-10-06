#ifndef WGPU_LAB_PIPELINE_H
#define WGPU_LAB_PIPELINE_H

#include <lab_buffer.h>
#include <lab_error.h>
#include <lab_gpu.h>
#include <lab_shader.h>
#include <lab_surface.h>
#include <lab_texture.h>

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <optional>
#include <string>
#include <vector>

namespace lab {

// returns the size in bytes of a wgpu::VertexFormat
constexpr uint64_t vertex_format_size(wgpu::VertexFormat format) {
  using enum wgpu::VertexFormat;
  switch (format) {
  case Uint8:
  case Sint8:
  case Unorm8:
  case Snorm8:
    return 1;
  case Uint8x2:
  case Sint8x2:
  case Unorm8x2:
  case Snorm8x2:
  case Uint16:
  case Sint16:
  case Unorm16:
  case Snorm16:
  case Float16:
    return 2;
  case Uint8x4:
  case Sint8x4:
  case Unorm8x4:
  case Snorm8x4:
  case Unorm8x4BGRA:
  case Uint16x2:
  case Sint16x2:
  case Unorm16x2:
  case Snorm16x2:
  case Float16x2:
  case Float32:
  case Uint32:
  case Sint32:
  case Unorm10_10_10_2:
  case Snorm10_10_10_2:
    return 4;
  case Uint16x4:
  case Sint16x4:
  case Unorm16x4:
  case Snorm16x4:
  case Float16x4:
  case Float32x2:
  case Uint32x2:
  case Sint32x2:
    return 8;
  case Float32x3:
  case Uint32x3:
  case Sint32x3:
    return 12;
  case Float32x4:
  case Uint32x4:
  case Sint32x4:
    return 16;
  }
  return 0;
}

// -------------------------------------------------------------------------------------------------
// What a pipeline renders into ---------------------------------------------------------------------

// The texture formats of a render target. A pipeline is built for exactly one
// combination and can only draw in passes onto a matching target.
struct TargetFormat {
  wgpu::TextureFormat color;
  wgpu::TextureFormat depth = wgpu::TextureFormat::Undefined; // Undefined: no depth buffer

  TargetFormat(const Surface& surface) : color{surface.format()}, depth{surface.depth_format()} {}
  TargetFormat(const Texture& texture) : color{texture.format()} {}
  TargetFormat(const Texture& color, const Texture& depth) : color{color.format()}, depth{depth.format()} {}
  TargetFormat(wgpu::TextureFormat color, wgpu::TextureFormat depth = wgpu::TextureFormat::Undefined)
      : color{color}, depth{depth} {}

  bool operator==(const TargetFormat&) const = default;
};

// -------------------------------------------------------------------------------------------------
// How vertex buffers are laid out -----------------------------------------------------------------

// One attribute of a vertex. Just naming the format is enough in most cases:
// attributes follow each other in memory and shader locations count up from 0.
struct VertexAttribute {
  static constexpr uint32_t next_location = ~uint32_t{0}; // one after the previous attribute
  static constexpr uint64_t next_offset = ~uint64_t{0};   // right behind the previous attribute

  wgpu::VertexFormat format;
  uint32_t location = next_location; // `@location(n)` in the shader
  uint64_t offset = next_offset;     // byte offset within the vertex

  VertexAttribute(wgpu::VertexFormat format, uint32_t location = next_location, uint64_t offset = next_offset)
      : format{format}, location{location}, offset{offset} {}
};

// The layout of one vertex buffer slot of a pipeline, made by `lab::vertex<T>()` or `lab::instance<T>()`
struct VertexLayout {
  std::vector<VertexAttribute> attributes;
  uint64_t stride = 0;
  wgpu::VertexStepMode step_mode = wgpu::VertexStepMode::Vertex;
};

namespace detail {
VertexLayout make_vertex_layout(std::initializer_list<VertexAttribute> attributes, uint64_t stride,
                                wgpu::VertexStepMode step_mode);
}

// Describes a buffer of T that is advanced once per vertex
// ```cpp
// struct MyVertex { float pos[2]; float color[3]; };
// lab::vertex<MyVertex>({Float32x2, Float32x3}) // position at @location(0), color at @location(1)
// ```
// Throws lab::Error if the attributes need more bytes than a T has.
template<GpuData T>
VertexLayout vertex(std::initializer_list<VertexAttribute> attributes) {
  return detail::make_vertex_layout(attributes, sizeof(T), wgpu::VertexStepMode::Vertex);
}

// Describes a buffer of T that is advanced once per instance
template<GpuData T>
VertexLayout instance(std::initializer_list<VertexAttribute> attributes) {
  return detail::make_vertex_layout(attributes, sizeof(T), wgpu::VertexStepMode::Instance);
}

// -------------------------------------------------------------------------------------------------
// What to draw --------------------------------------------------------------------------------------

// A vertex buffer for a draw call, any `lab::Buffer<T>` converts to it
struct VertexBufferRef {
  wgpu::Buffer buffer;
  uint64_t stride = 0;
  uint64_t count = 0;

  template<GpuData T>
  VertexBufferRef(const Buffer<T>& buffer) : buffer{buffer.handle()}, stride{sizeof(T)}, count{buffer.size()} {}
  VertexBufferRef(wgpu::Buffer buffer, uint64_t stride, uint64_t count)
      : buffer{std::move(buffer)}, stride{stride}, count{count} {}
};

// An index buffer for a draw call, `lab::Buffer<uint16_t>` and `lab::Buffer<uint32_t>` convert to it
struct IndexBufferRef {
  wgpu::Buffer buffer;
  wgpu::IndexFormat format = wgpu::IndexFormat::Undefined;
  uint64_t count = 0;

  IndexBufferRef() = default;
  IndexBufferRef(const Buffer<uint16_t>& buffer)
      : buffer{buffer.handle()}, format{wgpu::IndexFormat::Uint16}, count{buffer.size()} {}
  IndexBufferRef(const Buffer<uint32_t>& buffer)
      : buffer{buffer.handle()}, format{wgpu::IndexFormat::Uint32}, count{buffer.size()} {}
};

// Everything a draw call needs besides the pipeline: which buffers, which bind
// groups, how much of them. It only holds handles, so it is cheap to keep around
// and to reuse every frame.
// ```cpp
// lab::Draw triangle{.vertex_buffers = {vertices}};
// lab::Draw nodes{.vertex_buffers = {mesh, node_instances}, .index_buffer = mesh_indices, .bind_groups = {group}};
// ```
struct Draw {
  static constexpr uint32_t all = ~uint32_t{0};

  std::vector<VertexBufferRef> vertex_buffers = {}; // one per vertex buffer slot of the pipeline
  IndexBufferRef index_buffer = {};                 // optional
  std::vector<wgpu::BindGroup> bind_groups = {};    // `@group(n)` in the shader is bind_groups[n]

  uint32_t count = all;     // vertices (or indices, with an index buffer); `all`: as many as the buffers hold
  uint32_t instances = all; // `all`: as many as the per-instance buffers hold, 1 if there are none
  uint32_t first = 0;       // first vertex (or first index)
  uint32_t first_instance = 0;
  int32_t base_vertex = 0; // added to each index, only with an index buffer
};

// -------------------------------------------------------------------------------------------------
// Pipeline ------------------------------------------------------------------------------------------

// One resource of a bind group, see `Pipeline::bind_group()`
struct Binding {
  uint32_t binding; // `@binding(n)` in the shader
  wgpu::Buffer buffer = nullptr;
  uint64_t offset = 0;
  uint64_t size = wgpu::kWholeSize;
  wgpu::TextureView texture_view = nullptr;
  wgpu::Sampler sampler = nullptr;

  template<GpuData T>
  Binding(uint32_t binding, const Buffer<T>& buffer) : binding{binding}, buffer{buffer.handle()} {}
  Binding(uint32_t binding, wgpu::Buffer buffer, uint64_t offset = 0, uint64_t size = wgpu::kWholeSize)
      : binding{binding}, buffer{std::move(buffer)}, offset{offset}, size{size} {}
  Binding(uint32_t binding, const Texture& texture) : binding{binding}, texture_view{texture.view()} {}
  Binding(uint32_t binding, wgpu::TextureView view) : binding{binding}, texture_view{std::move(view)} {}
  Binding(uint32_t binding, wgpu::Sampler sampler) : binding{binding}, sampler{std::move(sampler)} {}
};

namespace detail {
// used by Pipeline and ComputePipeline
wgpu::BindGroup make_bind_group(const std::shared_ptr<GpuState>& gpu, const wgpu::BindGroupLayout& layout,
                                std::initializer_list<Binding> bindings, std::string_view label);
} // namespace detail

inline constexpr wgpu::BlendState alpha_blend{
    .color{.operation = wgpu::BlendOperation::Add,
           .srcFactor = wgpu::BlendFactor::SrcAlpha,
           .dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha},
    .alpha{.operation = wgpu::BlendOperation::Add,
           .srcFactor = wgpu::BlendFactor::One,
           .dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha},
};

// How a pipeline is set up. Only `target` has to be given.
// ```cpp
// lab::Pipeline pipeline(gpu, shader, {
//     .vertex_buffers = {lab::vertex<MyVertex>({Float32x2, Float32x3})},
//     .target = surface,
// });
// ```
struct PipelineDesc {
  // layout of each vertex buffer the shader reads, in the order they are passed to draw calls
  std::vector<VertexLayout> vertex_buffers = {};

  // the surface, texture or format this pipeline renders into
  TargetFormat target;

  wgpu::PrimitiveTopology topology = wgpu::PrimitiveTopology::TriangleList;
  wgpu::CullMode cull = wgpu::CullMode::None;

  // how the output is mixed with what is already in the target, nullopt replaces it
  std::optional<wgpu::BlendState> blend = alpha_blend;

  // Depth test, only used if the target has a depth buffer: a fragment is kept if its
  // depth compares like this to what is stored, and then stored itself if depth_write is set
  wgpu::CompareFunction depth_compare = wgpu::CompareFunction::Less;
  bool depth_write = true;

  // only needed if the shader has more than one entry point per stage
  std::string vertex_entry = {};
  std::string fragment_entry = {};

  // Bind group layouts. Leave empty to derive them from the shader (`layout: auto`),
  // which is what `Pipeline::bind_group()` builds on.
  std::vector<wgpu::BindGroupLayout> layouts = {};

  // last word on the raw descriptor, for everything this struct does not cover
  std::function<void(wgpu::RenderPipelineDescriptor&)> customize = {};

  // defaults to the label of the shader
  std::string label = {};
};

// A render pipeline: a shader plus the fixed state it is drawn with.
// It is immutable. What it draws (buffers, bind groups) is given per draw call.
class Pipeline {
public:
  // Throws lab::Error if WebGPU rejects the pipeline, e.g. because the vertex
  // layouts do not provide what the shader expects
  Pipeline(Gpu& gpu, const Shader& shader, PipelineDesc desc);

  Pipeline(Pipeline&&) = default;
  Pipeline& operator=(Pipeline&&) = default;

  // Creates the bind group for `@group(group)` of the shader
  // ```cpp
  // wgpu::BindGroup group = pipeline.bind_group(0, {{0, uniform_buffer}, {1, texture}});
  // ```
  // A bind group made this way belongs to this pipeline (unless PipelineDesc::layouts was given).
  // Throws lab::Error if the bindings do not match what the shader declares.
  wgpu::BindGroup bind_group(uint32_t group, std::initializer_list<Binding> bindings,
                             std::string_view label = {}) const;

  // Renders one whole frame with a single draw call, which is all a simple program needs:
  // ```cpp
  // while (lab::tick()) {
  //   pipeline.render_frame(surface, {.vertex_buffers = {vertices}});
  // }
  // ```
  // Returns false if the frame was skipped (see Surface::current_view).
  // For more than one draw call per frame, use `lab::RenderPass`.
  bool render_frame(Surface& surface, const Draw& draw) const;
  // for pipelines without vertex buffers
  bool render_frame(Surface& surface, uint32_t vertex_count, uint32_t instance_count = 1) const;

  const wgpu::RenderPipeline& handle() const { return pipeline; }
  const std::string& label() const { return name; }
  const TargetFormat& target() const { return target_format; }

  // stride and step mode of each vertex buffer slot, used to check draw calls
  struct Slot {
    uint64_t stride;
    wgpu::VertexStepMode step_mode;
  };
  const std::vector<Slot>& slots() const { return vertex_slots; }

private:
  std::shared_ptr<detail::GpuState> gpu;
  wgpu::RenderPipeline pipeline;
  TargetFormat target_format;
  std::vector<Slot> vertex_slots;
  std::string name;
};

} // namespace lab

#endif // WGPU_LAB_PIPELINE_H
