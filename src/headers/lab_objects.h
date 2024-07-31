#ifndef WGPU_LAB_OBJECTS_H
#define WGPU_LAB_OBJECTS_H

namespace lab {

// Forward declarations of public lab objects and corresponding handle types.
// The idea is to use these handles in maps to model relations between objects.
// See `LabWindowMap` in `lab_state.h` which maps `GLFWwindow*` to `Window*`

using Handle = void*; // any Handle

// lab structs -----------------------------------

struct Webgpu;
using WebgpuHandle = Webgpu*;

struct Window;
using WindowHandle = Window*;

struct Shader;
using ShaderHandle = Shader*;

struct Surface;
using SurfaceHandle = Surface*;

struct Pipeline;
using PipelineHandle = Pipeline*;

} // namespace lab

// external structs ------------------------------

struct GLFWwindow;
using GlfwWindowHandle = GLFWwindow*;

#endif // WGPU_LAB_OBJECTS_H
