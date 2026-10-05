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

You need a C++23 compiler, [CMake](https://cmake.org/) 3.28 or newer,
[Ninja](https://ninja-build.org/) and git.

```sh
git clone https://github.com/bv7dev/wgpu-lab.git
cd wgpu-lab
cmake --preset dev
cmake --build --preset dev
```

The first `cmake --preset dev` downloads a prebuilt Dawn (about 40 MB on Linux),
so there is no long Dawn build to wait for.

**Run sample executables:**

The samples end up at the top of the build directory, next to the shaders they load:

```sh
cd build/dev
./sample_vertex_buffer
```

For VS Code users, there's a shared `.vscode/launch.json` configuration file.
This setup allows you to build and run any `.cpp` source file that's located in the `samples/` directory,
simply by opening it in the editor and pressing `F5`. This runs the code in debug mode (set breakpoints and step through the code to learn how it works).

To get started, you can add your own `.cpp` file, tinker around and step through the code. Use CMake Tools to reconfigure the project after adding new files.

**Run the tests:**

```sh
ctest --preset dev              # everything; each sample opens its window for a second
ctest --preset dev -LE samples  # only the tests that need no window
```

Any sample can be run unattended by setting `LAB_EXIT_AFTER_FRAMES`, for example
`LAB_EXIT_AFTER_FRAMES=120 ./sample_texture` closes its window after 120 frames.

### Linux

Wayland and X11 are both supported. If GLFW 3.4 or newer is installed
(`glfw` on Arch, `libglfw3-dev` on recent Debian/Ubuntu) it is used, otherwise GLFW is built
from source and needs its build dependencies, for example on Debian/Ubuntu:

```sh
sudo apt install libwayland-dev libxkbcommon-dev xorg-dev
```

### Windows

Install [Visual Studio](https://visualstudio.microsoft.com/vs/community/) with the
"Desktop development with C++" workload (it includes MSVC, CMake and Ninja) and run the
commands above from a "Developer PowerShell for VS".
Alternatively, open the folder in [VS Code](https://code.visualstudio.com/) with the
recommended extensions and pick the `dev` preset.

### Mac (help wanted)

### Choosing where Dawn comes from

| `-DLAB_DAWN=` | What happens |
|---|---|
| `prebuilt` (default) | downloads the release archive that Dawn's CI publishes for the pinned version |
| `source` | fetches and builds Dawn itself (takes a while, needs Python for Dawn's build scripts) |
| `system` | uses a Dawn you installed yourself, found through `find_package(Dawn)` |

The pinned Dawn version is set at the top of `cmake/LabDawn.cmake`.

### Dependencies
The library only depends on [WebGPU Dawn](https://dawn.googlesource.com/dawn) and
[GLFW](https://www.glfw.org/) for windowing.
wgpu-lab also makes heavy use of the C++ STL (see `src/extra/lab_public.h`).
However, to build all of the sample executables, the libraries
[GLM](https://github.com/g-truc/glm) and [tinygltf](https://github.com/syoyo/tinygltf)
are downloaded as well.


## Roadmap
- [x] address build system issues
- [ ] re-design render pipeline (too chaotic at the moment)
- [ ] replace render_frame() function by smaller, composable mechanisms
- [ ] add compute pipeline support
- [ ] add emscripten support for WebAssembly
- [ ] unify and finalize lab API 
- [ ] write documentation and tests 
- [ ] release stable 1.0 version
