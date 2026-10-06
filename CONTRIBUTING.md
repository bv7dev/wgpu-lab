# Contributing to wgpu-lab

Contributions are welcome: bug reports, samples, platform support, documentation.

## Building and testing

```sh
cmake --preset dev
cmake --build --preset dev
ctest --preset dev              # everything; samples and window tests open windows briefly
ctest --preset dev -LE display  # only the tests that need no window
```

`dev-asan` is the same build with AddressSanitizer and UBSan, worth a run after changes to
object lifetimes:

```sh
cmake --preset dev-asan && cmake --build --preset dev-asan && ctest --preset dev-asan
```

The web build needs [Emscripten](https://emscripten.org/) on the PATH:

```sh
emcmake cmake --preset web && cmake --build --preset web
cd build/web/site && python -m http.server 8000   # http://localhost:8000/
```

To see what the samples render without watching them, let them save a frame each:

```sh
cd build/dev
LAB_CAPTURE_DIR=../captures LAB_EXIT_AFTER_FRAMES=60 ./07_graph
```

## Where things are

| Path | What |
|---|---|
| `include/` | the public API, `include/lab` includes all of it |
| `src/` | the implementation, `src/lab_detail.h` holds the state behind the public objects |
| `src/platform/` | everything that differs between window systems |
| `samples/` | one program per `.cpp` file, shaders in `samples/shaders/`, named after their sample |
| `tests/` | doctest tests, they render into textures and compare pixels |
| `cmake/LabDawn.cmake` | the pinned Dawn version and where it is downloaded from |

## Conventions

- Format code with clang-format 23.1.1 (`pipx run clang-format==23.1.1 -i <files>`), CI checks it.
  Other versions format a few constructs differently.
- A lab class exists where the lab adds state. Where a `wgpu::` handle is all there is
  (samplers, bind groups, texture views), a function that returns the handle is enough.
- Lab objects do not hold references to each other. They share state through `shared_ptr`
  (see `src/lab_detail.h`), which keeps them movable and lets programs destroy them in any order.
- Every object exposes the WebGPU object it wraps through `handle()`.
- Mistakes the lab can detect are reported with `detail::fail(label, message)`, which throws
  `lab::Error`. Say what is wrong, with the numbers, and what to look at.
- A new feature comes with a test, and with a sample if it is something to learn from.
- One logical change per commit, with a message that says why.

## Supporting another platform

Creating a surface for a window is the only platform specific code, see
`src/platform/lab_surface_glfw.cpp`. macOS needs a `CAMetalLayer` for the window there
(`wgpu::SurfaceSourceMetalLayer`), and is the most wanted contribution.

## Updating Dawn

`cmake/LabDawn.cmake` names one Dawn release: its tag, its commit and the hashes of the archives.
Pick a newer release from <https://github.com/google/dawn/releases>, update those values, build
and run the tests. `webgpu_cpp.h` still changes between releases, so expect a few renames.
