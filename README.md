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


## Getting Started

### windows

**setup:**
1. install [Visual Studio](https://visualstudio.microsoft.com/vs/community/) for MSVC compiler and CMake, or a C++ compiler and build tools of your choice (configure it yourself and feel free to share your setup by creating an issue or a pull request)
1. install [VS Code](https://code.visualstudio.com/) editor (optional - this project is configured to work well within VS Code)  
   open VS Code and install recommended extensions (a pop-up should appear)  
   look into the extensions tab to see if extensions `C/C++`, `CMake` and `CMake Tools` are installed 
1. install [Python](https://www.python.org/downloads/) which is required to download dependencies in wgpu-lab and dawn

**build:**
1. clone this repository:
   ```sh
   git clone https://github.com/bv7dev/wgpu-lab.git
   ```
   or alternatively, download a release build
1. open the cloned directory or the unzipped release in VS Code (File -> Open Folder...)
1. hit the `⚙ Build` button in Code's bottom toolbar (provided by `CMake Tools` extension) or use `CMake` manually to configure and build

Only the first build takes a very long time for downloading and building dawn


**run and debug sample executables:**

For VS Code users, there's a shared `.vscode/launch.json` configuration file.
This setup allows you to build and run any `.cpp` source file that's located in the `samples/` directory,
simply by opening it in the editor and pressing `F5` which runs the code in debug mode (set breakpoints and step through code to learn how it works).

To get started, you can add your own `.cpp` file and tinker around and step through the code.

### Dependencies
The library currently only depends on [WebGPU Dawn](https://dawn.googlesource.com/dawn) and uses
[GLFW](https://www.glfw.org/) for windowing, which already comes included with dawn.
wgpu-lab also makes heavy use of the C++ STL (see `src/extra/lab_public.h`).
However, to build all of the sample executables, the libraries
[GLM](https://github.com/g-truc/glm) and [tinygltf](https://github.com/syoyo/tinygltf)
are also included.


## Roadmap
- [x] address build system issues from WIP section
- [ ] re-design render pipeline (too chaotic at the moment)
- [ ] replace render_frame function by smaller, composable mechanisms
- [ ] add compute pipeline support
- [ ] add emscripten support for WebAssembly
- [ ] unify and finalize lab API and release stable 1.0 version
