#include "lab_pipeline.h"

#include <format>
#include <iostream>
#include <sstream>

namespace lab {

Pipeline::Pipeline(Shader& sh, Webgpu& wg, bool now) : shader{sh}, webgpu{wg} {
  if (now) create();
}

wgpu::RenderPipeline Pipeline::create(wgpu::ShaderModule shaderModule) {
  if (config.label.empty()) {
    config.label = std::format("Default Pipeline({} on {})", shader.label, webgpu.label);
  }
  std::cout << "Info: WGPU: Create: " << config.label << std::endl;

  wgpu::BlendState blendState = {{
      .color = config.blendColor,
      .alpha = config.blendAlpha,
  }};

  config.colorTarget.format = webgpu.capabilities.formats[0];
  config.colorTarget.blend = &blendState;

  config.fragmentState.module = shaderModule;
  config.fragmentState.targetCount = 1;
  config.fragmentState.targets = &config.colorTarget;

  config.vertexState.module = shaderModule;

  wgpu::RenderPipelineDescriptor pipelineDesc = {{
      .label = config.label.c_str(),
      .vertex = config.vertexState,
      .primitive = config.primitiveState,
      .multisample = config.multisampleState,
      .fragment = &config.fragmentState,
  }};

  wgpu_pipeline = webgpu.device.createRenderPipeline(pipelineDesc);
  return wgpu_pipeline;
}

void Pipeline::create() {
  wgpu::ShaderModule shaderModule = shader.transfer(webgpu.device);
  create(shaderModule);
  shaderModule.release();
}

bool Pipeline::render_frame(Surface& surface) { return render_func(*this, surface); }

Pipeline::~Pipeline() {
  if (wgpu_pipeline) {
    std::cout << "Info: Release Pipeline" << std::endl;
    wgpu_pipeline.release();
    wgpu_pipeline = nullptr;
  }
}

} // namespace lab
