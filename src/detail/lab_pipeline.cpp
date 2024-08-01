#include <objects/lab_pipeline.h>

#include <format>
#include <iostream>
#include <sstream>

namespace lab {

void Pipeline::finalize_config(wgpu::ShaderModule shaderModule) {
  if (label.empty()) {
    label = std::format("Default Pipeline({} on {})", shader.label, webgpu.label);
  }

  if (vertex_buffer) {
    config.vertexAttribute_0.shaderLocation = 0;
    config.vertexAttribute_0.format = wgpu::VertexFormat::Float32x2;

    config.vertexBufferLayout.attributeCount = 1;
    config.vertexBufferLayout.attributes = &config.vertexAttribute_0;

    config.vertexBufferLayout.arrayStride = 2 * sizeof(float);
    config.vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

    config.vertexState.bufferCount = 1;
    config.vertexState.buffers = &config.vertexBufferLayout;
  }

  config.colorTarget.format = webgpu.capabilities.formats[0];
  config.colorTarget.blend = &config.blendState;

  // todo: make number of color targets configurable
  config.fragmentState.targetCount = 1;
  config.fragmentState.targets = &config.colorTarget;

  config.vertexState.module = shaderModule;
  config.fragmentState.module = shaderModule;
}

wgpu::RenderPipeline Pipeline::transfer() const {
  wgpu::RenderPipelineDescriptor pipelineDesc = {{
      .label = label.c_str(),
      .vertex = config.vertexState,
      .primitive = config.primitiveState,
      .multisample = config.multisampleState,
      .fragment = &config.fragmentState,
  }};

  return webgpu.device.createRenderPipeline(pipelineDesc);
}

Pipeline::~Pipeline() {
  if (wgpu_pipeline) {
    wgpu_pipeline.release();
    wgpu_pipeline = nullptr;
  }
}

} // namespace lab
