# WebGPU Lab
wgpu-lab is a library designed for rapid prototyping of native WebGPU applications in C++.
Its main goal is to provide convenient wrappers and intuitive tools for working with
[WebGPU Dawn](https://dawn.googlesource.com/dawn),
minimizing boilerplate code while remaining flexible and customizable.

I’m developing this library in my free time as I learn the fundamentals of WebGPU.
My hope is that it will serve the open source community as a useful resource for learning
and as a foundation for building creative projects.

Please note that wgpu-lab is in an early, heavily experimental stage,
and the API is likely to undergo significant changes.

**Contributions are welcome!**


## Simple Usage Sample 
```c++
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
      //         X      Y                R     G     B
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


## How to Build and Run samples
wgpu-lab is intended to be included into projects as a Git submodule.
Integration with other build systems should be straightforward.

The library and sample executables are built using CMake. If you're developing the library or
experimenting with the samples, I recommend using the `CMake Tools` extension for Visual Studio Code
provided by Microsoft. The build system is a work in progress and may evolve over time.

For VS Code users, there's a shared `.vscode/launch.json` configuration file inside this repository.
This setup allows you to build and run any `.cpp` file located in the `./samples` directory
simply by opening it in the editor and pressing `F5`.

### Dependencies
The library currently only depends on [WebGPU Dawn](https://dawn.googlesource.com/dawn) and uses
[GLFW](https://www.glfw.org/) for windowing, which already comes included with dawn.
It also makes heavy use of the C++ STL (see `./src/extra/lab_public.h`).
However, to build all of the sample executables, you'll also need to add
[GLM](https://github.com/g-truc/glm) and [tinygltf](https://github.com/syoyo/tinygltf)
to the `./libs` directory.

### Work in Progress
- Based on previous unpleasant experiences, I've decided not to add wgpu-lab's dependencies as Git submodules for now.
  Instead, you currently need to manually clone the dependencies into a `./libs` directory under this project's root directory.
  I'm still considering whether to add them as submodules or implement a script to automate fetching dependencies.
- At the moment, Dawn is built as a `.dll` file but isn't automatically copied into wgpu-lab's build folder.
  After the first build, you'll need to manually copy `webgpu_dawn.dll` into the `./build` directory for now.
  Soon, this should be automated and there should be an option to choose between static and dynamic linking with Dawn.
- So far the entire project was only tested on windows, using dawn branch
  [chromium/6670](https://dawn.googlesource.com/dawn/+/refs/heads/chromium/6670)


## Roadmap
- [ ] address build system issues from WIP section
- [ ] re-design render pipeline (too chaotic at the moment)
- [ ] replace render_frame function by smaller, composable mechanisms
- [ ] add compute pipeline support
- [ ] add emscripten support for WebAssembly
- [ ] unify and finalize lab API and release stable 1.0 version
