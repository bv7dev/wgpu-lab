#ifndef WGPU_LAB_PIPELINE_H
#define WGPU_LAB_PIPELINE_H

#include "lab_shader.h"
#include "lab_surface.h"
#include "lab_webgpu.h"

#include <string>

namespace lab {

struct Pipeline {
  Shader& shader;
  Webgpu& webgpu;

  wgpu::RenderPipeline wgpu_pipeline;

  Pipeline(Shader&, Webgpu&);

  bool render_frame(Surface& surface);

  ~Pipeline();
};

} // namespace lab

#endif // WGPU_LAB_PIPELINE_H
