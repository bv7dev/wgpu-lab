#include "lab_pipeline.h"

#include <format>
#include <iostream>
#include <sstream>

namespace lab {

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

  wgpu::MultisampleState multisampleState = {{
      .count = 1,
      .mask = ~0u,
  }};

  wgpu::RenderPipelineDescriptor pipelineDesc = {{
      .label = config.label.c_str(),
      .vertex = config.vertexState,
      .primitive = config.primitiveState,
      .multisample = multisampleState,
      .fragment = &config.fragmentState,
  }};

  wgpu_pipeline = webgpu.device.createRenderPipeline(pipelineDesc);
  return wgpu_pipeline;
}

Pipeline::~Pipeline() {
  if (wgpu_pipeline) {
    std::cout << "Info: Release Pipeline" << std::endl;
    wgpu_pipeline.release();
    wgpu_pipeline = nullptr;
  }
}

} // namespace lab
