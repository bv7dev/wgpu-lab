# Migrating from wgpu-lab 0.2

Version 0.3 revived the lab on a current Dawn with the old API intact.
The version after it redesigned the API. This page lists what changed and why,
so that code written against 0.2 or 0.3 can be carried over.

## What stayed

- `#include <lab>` and `while (lab::tick()) { ... }`
- a handful of RAII objects set up in a few lines
- `wgpu::` types are used directly, nothing of WebGPU is hidden

## The ideas behind the changes

1. **A pipeline is immutable and made for one target.** There is no `finalize()`
   anymore and no format shared behind the scenes. A pipeline is complete when its
   constructor returns, or the constructor throws.
2. **Buffers and bind groups belong to the draw call, not to the pipeline.**
   A pipeline describes the *layout* of its vertex buffers. Which buffers are drawn
   is said per draw call, in a `lab::Draw`. Two pipelines can share a mesh, and
   one pipeline can draw many meshes.
3. **Bind group layouts come from the shader.** The shader already declares its
   bindings, so `pipeline.bind_group(0, {{0, uniforms}})` is all there is to write.
4. **Rendering is composable.** `lab::RenderPass` takes any number of draw calls,
   `lab::Frame` any number of passes, and a pass can render into a `lab::Texture`
   instead of a window.
5. **Objects do not refer to each other.** They share reference-counted state, so
   they can be destroyed in any order and moved around (into a `std::vector`, for example).
6. **No threads inside the library.**
7. **Mistakes are exceptions.** `lab::Error` is thrown where the mistake is made
   and names the object. Errors that WebGPU reports later are thrown by the next
   `lab::tick()`.

## Renamed

| 0.2 | now |
|---|---|
| `lab::Webgpu webgpu("label")` | `lab::Gpu gpu;` (options: `lab::Gpu gpu({.label = "label"})`) |
| `webgpu.device`, `.queue`, `.instance`, `.adapter` | `gpu.device()`, `gpu.queue()`, `gpu.instance()`, `gpu.adapter()` |
| `buffer.wgpu_buffer`, `surface.wgpu_surface`, `pipeline.wgpu_pipeline`, `texture.wgpu_texture`, `window.glfw_window_handle` | `handle()` on every object |
| `lab::Surface surface(window, webgpu)` | `lab::Surface surface(gpu, window)` (the Gpu always comes first) |
| `lab::Shader shader("label", "file.wgsl")` | `lab::Shader shader(gpu, "file.wgsl")`, compiled right away |
| `shader.source = "..."` | `lab::Shader::from_source(gpu, "...")` |
| `lab::Buffer<T> buffer("label", data, usage, webgpu)` | `lab::Buffer<T> buffer(gpu, data, usage, "label")` |
| `window.set_key_callback(f)`, `set_resize_callback(f)` | `window.on_key(f)`, `window.on_resize(f)` |
| `window.ratio()` (height / width) | `window.aspect()`, `surface.aspect()` (width / height) |
| `texture.to_device(pixels)` | `texture.write(pixels)` |
| `texture.create_view()` | `texture.view()` |
| `lab::ModKey::shift` as a plain enum | `enum class`, combined with `\|` and tested with `&` |
| `include <extra/...>`, `<objects/...>` | `<lab>`, or single headers such as `<lab_buffer.h>` |

## Replaced

**Setting up a pipeline**

```c++
// 0.2
lab::Pipeline pipeline(shader, webgpu);
pipeline.add_vertex_buffer(vertex_buffer);
pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 0);
pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x3, 1);
pipeline.add_uniform_buffer(uniform_buffer, 0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
pipeline.add_texture(texture, 1);
pipeline.config.primitiveState.topology = wgpu::PrimitiveTopology::LineStrip;
pipeline.finalize();

// now
lab::Pipeline pipeline(gpu, shader, {
    .vertex_buffers = {lab::vertex<MyVertex>({Float32x2, Float32x3})},
    .target = surface,
    .topology = wgpu::PrimitiveTopology::LineStrip,
});
lab::Draw draw{
    .vertex_buffers = {vertex_buffer},
    .bind_groups = {pipeline.bind_group(0, {{0, uniform_buffer}, {1, texture}})},
};
```

A per-instance buffer is declared with `lab::instance<T>({...})` in place of
`add_vertex_buffer(buffer, wgpu::VertexStepMode::Instance)`, an index buffer is
set as `Draw::index_buffer`. Anything `PipelineDesc` has no field for can be set
on the raw descriptor in `PipelineDesc::customize`.

**Rendering**

```c++
// 0.2
pipeline.render_frame(surface, 3, 1);          // or {3, 1}
pipeline.render_config.renderPassColorAttachment.clearValue = {0.2, 0.2, 0.2, 1.0};

// now: counts come from the buffers of the draw call
pipeline.render_frame(surface, draw);
pipeline.render_frame(surface, 3);             // pipelines without vertex buffers

// several draw calls, another clear color
lab::RenderPass pass(surface, {.clear = wgpu::Color{0.2, 0.2, 0.2, 1.0}});
pass.draw(pipeline, draw);
pass.draw(other_pipeline, other_draw);
```

Custom render functions (`Pipeline::render_func`) and
`get_current_render_texture_view()` are no longer needed: use `lab::RenderPass`,
or `surface.current_view()` and `surface.present()` to do it all by hand.

**Reading and writing buffers**

```c++
// 0.2: the mapped memory was handed to another thread
auto thread = buffer.to_device([](lab::MappedVRAM<int> vmap) { vmap.push(1); }, 256, usage);
auto reader = buffer.from_device([](lab::MappedVRAM<const int> vmap) { ... });

// now
lab::Buffer<int> buffer(gpu, 256, usage, [](std::span<int> mapped) { mapped[0] = 1; });
std::vector<int> values = buffer.read();                           // waits for the gpu
buffer.read_async([](std::span<const int> values) { ... });        // or: called by a later tick()
```

The lab adds the usage flags that `write()` and `read()` need, so
`wgpu::BufferUsage::Uniform` is enough where 0.2 wanted `Uniform | CopyDst`.

**Resizing.** A surface follows the size of its window on its own. Calls to
`surface.reconfigure()` in resize callbacks can be deleted.

**Chrono literals.** `<lab>` no longer injects `using namespace std::chrono_literals`,
write it yourself where you want `100ms`. The macro `LAB_USER_DISABLE_CHRONO_LITERALS` is gone.

## Removed without replacement

- `lab::init_lab()`, `lab::state` and the handle typedefs (`WindowHandle`, ...)
- `Webgpu::capabilities`
- `Shader::transfer()`, `Pipeline::transfer()`, `Texture::transfer()`:
  the objects own what they create, `handle()` gives access
- `Buffer<std::string>` and other element types that are not plain data
