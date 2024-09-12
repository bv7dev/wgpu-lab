## wgpu-lab

is a library for rapid prototyping of native WebGPU applications.
Main purpose is to provide convenience wrappers and tools for working with WebGPU Dawn.
Currently the library is in a very early, heavily experimental stage of development.
Expect the API to change a lot until it is ready for a `1.0` release.


### How to build the library and run sample executables

wgpu-lab is intended to be included into projects as a git submodule. 
Any other way to link it should be fine. Integration into other build systems should be trivial.
Feel free to contribute.

CMake is used to build the library and sample executables. 
For developing the library I recommend using the `CMake Tools` extension for Visual Studio Code, provided by Microsoft. 
The build system is work in progress and likely to change a lot in the future.

If using VS Code, there is a shared `.vscode/launch.json` which enables to conveniently 
build and run any `test_(...).cpp` or `sample_(...).cpp` file by simply opening it in 
the editor and pressing `F5`


### Simple Example

```cpp
#include <lab>

int main() {
  lab::Webgpu webgpu("My WebGPU Context");
  lab::Shader shader("My Shader Module", "shaders/my_shader.wgsl");

  lab::Window window("Vertex Buffer Demo", 640, 400);
  lab::Surface surface(window, webgpu);

  std::vector<float> vertex_data = {-0.5f,  -0.5f, 1.0f, +0.5f,  -0.5f, 0.6f, +0.0f,  0.5f, 0.2f,
                                    -0.55f, -0.5f, 1.0f, -0.05f, +0.5f, 0.6f, -0.55f, 0.5f, 0.2f};

  lab::Buffer<float> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  lab::Pipeline pipeline(shader, webgpu);

  pipeline.add_vertex_buffer(vertex_buffer.wgpu_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0); // vec2 position
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32, 1);   // color

  pipeline.finalize();

  while (lab::tick()) {
    pipeline.render_frame(surface, {3, 1});
  }
}

```
