# Changelog

## 0.5.0

### Added
- The samples run in the browser: `emcmake cmake --preset web` builds a page per sample
  and a gallery, which a GitHub Pages workflow publishes. The lab uses Dawn's Emscripten
  port of the same Dawn release as the native build, and the contrib.glfw3 port for
  windows. `while (lab::tick())` stays as it is: on the web, `tick()` waits for the
  browser's next animation frame.
- `LAB_WINDOW_SYSTEM=x11|wayland` on Linux.

### Fixed
- The depth buffer of a surface lagged a frame behind after the window was resized.

## 0.4.0

The API was redesigned. [docs/migrating-from-0.2.md](docs/migrating-from-0.2.md) explains
what changed and how to carry code over.

### Added
- `lab::RenderPass` and `lab::Frame`: any number of draw calls per pass and passes per frame,
  onto a window or into a texture.
- `lab::Draw`: the buffers, bind groups and counts of a draw call, reusable every frame.
- `lab::ComputePipeline` and `lab::ComputePass`.
- Depth buffers (`SurfaceOptions::depth`, `PassOptions::depth`) and samplers (`lab::sampler`).
- `Buffer::read()`, `Buffer::read_async()`, `Texture::read()`, `Texture::save_png()`.
- Mouse input, and polling for keyboard and mouse state next to the callbacks.
- `lab::Error`: mistakes are reported as exceptions that name the object, where they are made.
- `lab::log` with levels and a replaceable sink, `LAB_LOG`.
- `lab::find_file()`, `lab::delta_seconds()`.
- `LAB_CAPTURE_DIR`: every window saves one frame as a PNG.
- Samples `10_house` (3D model, depth, camera), `11_boids` (compute) and `12_offscreen`.
- Tests that render into textures and compare pixels.

### Changed
- `lab::Webgpu` is now `lab::Gpu`, and it is the first argument wherever one is needed.
- `lab::Pipeline` is immutable and built for one render target. Vertex layouts are described
  with `lab::vertex<T>()` and `lab::instance<T>()`, bind groups are made by `pipeline.bind_group()`.
- Every object gives access to the WebGPU object it wraps through `handle()`.
- Objects can be moved, and destroyed in any order.
- A surface follows the size of its window on its own and prefers an 8-bit format.
- Shaders are compiled when they are created and found from the working directory or from next
  to the executable.
- The samples are a numbered sequence, one concept each.
- Public headers live in `include/`, flat: `<lab>`, or single ones such as `<lab_buffer.h>`.

### Removed
- `Pipeline::finalize()`, `add_vertex_buffer()`, `add_uniform_buffer()` and the other `add_*` functions,
  `Pipeline::config`, `render_config` and `render_func`.
- `MappedVRAM` and the thread based `to_device()` / `from_device()`.
- `lab::init_lab()`, `lab::state`, `Webgpu::capabilities`.

## 0.3.0

wgpu-lab builds again, and for the first time on Linux.

### Added
- Linux support, on Wayland and X11.
- CMake presets, tests (`ctest`), CI on Linux and Windows.
- `LAB_EXIT_AFTER_FRAMES` to run a program unattended.

### Changed
- Dawn is pinned to a release from October 2026 and downloaded as a prebuilt library by default
  (`-DLAB_DAWN=prebuilt|source|system`). The Python scripts are gone, as is the long first build.
- C++23 and CMake 3.28 are required.
- `<lab>` no longer injects `using namespace std::chrono_literals`.
- A missing shader file is an error.
- Without a GPU, a software adapter is used.

### Fixed
- Crash at exit on Wayland: the display connection was closed while the GPU still used it.
- `Pipeline::add_uniform_buffer()` ignored its binding index.
- Wrong arguments for indexed drawing.
- Vertex format sizes were looked up with outdated enum values.
- The surface was sized in window coordinates instead of pixels (wrong on scaled displays).
- `Buffer` kept a dangling label pointer and accepted element types that are not plain data.
- `sample_uniform_buffer` never exited, `test_windows` destroyed objects twice.

## 0.2.0

Dependencies are fetched automatically when CMake configures the project.

## 0.1.0-alpha

First public release.
