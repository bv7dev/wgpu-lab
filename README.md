# wgpu lab
wgpu-lab is a library designed for rapid prototyping of native `WebGPU` applications.
Its primary purpose is to provide convenient wrappers and intuitive tools for 
working with [WebGPU Dawn](https://dawn.googlesource.com/dawn).

Please note that this library is in an early, heavily experimental stage, 
and the API is expected to undergo significant changes.

**Contributions are welcome!**

## Building the Library and Running Sample Executables
wgpu-lab is intended to be included into projects as a Git submodule.
Integration with other build systems should be straightforward.

The library and sample executables are built using `CMake`.
If you're developing the library or experimenting with the samples, I recommend using
the `CMake Tools` extension for Visual Studio Code, provided by Microsoft.
The build system is still a work in progress and may evolve over time.

For Visual Studio Code users, there's a shared `.vscode/launch.json` configuration file.
This setup allows you to build and run any `.cpp` file located inside the `./samples/`
directory simply by opening it in the editor and pressing `F5`.


## Simple Example - Vertex Buffer with Vertex Colors

```cpp
#include <lab>

struct MyVertexFormat {
  float pos[2];
  float color[3];
};

int main() {
  lab::Webgpu webgpu("My WebGPU Context");
  lab::Shader shader("My Shader", "shaders/draw_colored.wgsl");
  lab::Pipeline pipeline(shader, webgpu);

  std::vector<MyVertexFormat> vertex_data = {
      //         x      y                r     g     b
      {.pos = {-0.5f, -0.5f}, .color = {0.8f, 0.2f, 0.2f}},
      {.pos = {+0.5f, -0.5f}, .color = {0.8f, 0.8f, 0.2f}},
      {.pos = {+0.0f, +0.5f}, .color = {0.2f, 0.8f, 0.4f}},
  };
  lab::Buffer vertex_buffer("My Vertex Buffer", vertex_data, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 0); // position
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x3, 1); // color
  pipeline.finalize();

  lab::Window window("Hello Triangle", 640, 400);
  lab::Surface surface(window, webgpu);

  while (lab::tick()) {
    pipeline.render_frame(surface, 3, 1);
    lab::sleep(50ms);
  }
}
```
