#ifndef WGPU_LAB_PIPELINE_H
#define WGPU_LAB_PIPELINE_H

#include "lab_shader.h"
#include "lab_surface.h"
#include "lab_webgpu.h"

#include <string>

namespace lab {

struct Pipeline {
  Webgpu& webgpu;
  Shader& shader;
  Surface& surface;

  wgpu::RenderPipeline wgpu_pipeline;

  Pipeline(Webgpu&, Shader&, Surface&);

  void render_frame();

  ~Pipeline();
};

} // namespace lab

#endif // WGPU_LAB_PIPELINE_H
