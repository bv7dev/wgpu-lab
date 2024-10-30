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

  lab::Pipeline pipeline(shader, webgpu); // the rendering pipeline

  // colored triangle data
  std::vector<MyVertexFormat> vertex_data = {
      //         X      Y                R     G     B
      {.pos = {-0.5f, -0.5f}, .color = {0.8f, 0.2f, 0.2f}},
      {.pos = {+0.5f, -0.5f}, .color = {0.8f, 0.8f, 0.2f}},
      {.pos = {+0.0f, +0.5f}, .color = {0.2f, 0.8f, 0.4f}},
  };

  // vertex buffer (sends copy of data to GPU memory)
  lab::Buffer vertex_buffer("My Vertex Buffer", vertex_data, webgpu);

  // pipeline needs to know about buffers and their memory layouts (vertex attributes)
  pipeline.add_vertex_buffer(vertex_buffer);
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x2, 0); // position
  pipeline.add_vertex_attrib(wgpu::VertexFormat::Float32x3, 1); // color
  pipeline.finalize();                                          // make ready for rendering

  lab::Window window("Hello Triangle", 640, 400);

  lab::Surface surface(window, webgpu); // surface to render onto

  // main application loop
  while (lab::tick()) {
    pipeline.render_frame(surface, 3, 1); // 3 vertices, 1 instance
  }
}
```


## Getting Started

### Windows

**Setup:**
1. Install [Visual Studio](https://visualstudio.microsoft.com/vs/community/) for MSVC compiler and CMake, or a C++ compiler and build tools of your choice (configure it yourself and feel free to share your setup by creating an issue or a pull request)
1. Install [VS Code](https://code.visualstudio.com/) (optional - this project is configured to work well within VS Code)  
   open VS Code and install recommended extensions (a pop-up should appear)  
   look into the extensions tab to see if `C/C++`, `CMake` and `CMake Tools` are installed 
1. Install [Python](https://www.python.org/downloads/) which is required to download dependencies in wgpu-lab and dawn

**Build:**
1. Clone this repository:
   ```sh
   git clone https://github.com/bv7dev/wgpu-lab.git
   ```
   or alternatively, download a release build
1. Open the cloned directory or the unzipped release in VS Code (File -> Open Folder...)
1. Hit the `⚙ Build` button in Code's bottom toolbar (provided by `CMake Tools` extension) or use `CMake` manually to configure and build

The first build takes a long time for downloading, generating and building dawn.

### Linux (coming soon)

### Mac (help wanted)

**Run sample executables:**

For VS Code users, there's a shared `.vscode/launch.json` configuration file.
This setup allows you to build and run any `.cpp` source file that's located in the `samples/` directory,
simply by opening it in the editor and pressing `F5`. This runs the code in debug mode (set breakpoints and step through the code to learn how it works).

To get started, you can add your own `.cpp` file, tinker around and step through the code. Use CMake Tools to reconfigure the project after adding new files.

### Dependencies
The library currently only depends on [WebGPU Dawn](https://dawn.googlesource.com/dawn) and uses
[GLFW](https://www.glfw.org/) for windowing, which already comes included with dawn.
wgpu-lab also makes heavy use of the C++ STL (see `src/extra/lab_public.h`).
However, to build all of the sample executables, the libraries
[GLM](https://github.com/g-truc/glm) and [tinygltf](https://github.com/syoyo/tinygltf)
are also included.


## Roadmap
- [x] address build system issues
- [ ] re-design render pipeline (too chaotic at the moment)
- [ ] replace render_frame() function by smaller, composable mechanisms
- [ ] add compute pipeline support
- [ ] add emscripten support for WebAssembly
- [ ] unify and finalize lab API 
- [ ] write documentation and tests 
- [ ] release stable 1.0 version
