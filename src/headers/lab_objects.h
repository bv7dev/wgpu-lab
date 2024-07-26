#ifndef WGPU_LAB_OBJECTS_H
#define WGPU_LAB_OBJECTS_H

#include "lab_shader.h"
#include "lab_webgpu.h"
#include "lab_window.h"

#include "lab_pipeline.h"
#include "lab_surface.h"

namespace lab {

using HShader = Shader*;
using HWebgpu = Webgpu*;
using HWindow = Window*;

using HPipeline = Pipeline*;
using HSurface = Surface*;

} // namespace lab

#endif // WGPU_LAB_OBJECTS_H
