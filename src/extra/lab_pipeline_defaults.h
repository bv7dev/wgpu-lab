#include <dawn/webgpu_cpp.h>

namespace lab::pipeline_defaults {

struct InitConfig {
  wgpu::BlendState blendState = {
      .color =
          {
              .operation = wgpu::BlendOperation::Add,
              .srcFactor = wgpu::BlendFactor::SrcAlpha,
              .dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha,
          },
      .alpha =
          {
              .operation = wgpu::BlendOperation::Add,
              .srcFactor = wgpu::BlendFactor::Zero,
              .dstFactor = wgpu::BlendFactor::One,
          },
  };

  wgpu::ColorTargetState colorTarget = {
      .writeMask = wgpu::ColorWriteMask::All,
  };

  wgpu::FragmentState fragmentState = {
      .entryPoint = "fs_main",
  };

  wgpu::VertexState vertexState = {
      .entryPoint = "vs_main",
  };

  wgpu::PrimitiveState primitiveState = {
      .topology = wgpu::PrimitiveTopology::TriangleList,
      .stripIndexFormat = wgpu::IndexFormat::Undefined,
      .frontFace = wgpu::FrontFace::CCW,
      .cullMode = wgpu::CullMode::None,
  };

  wgpu::MultisampleState multisampleState = {
      .count = 1,
      .mask = ~0u,
  };
};

struct RenderConfig {
  wgpu::RenderPassColorAttachment renderPassColorAttachment = {
      .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
      .loadOp = wgpu::LoadOp::Clear,
      .storeOp = wgpu::StoreOp::Store,
      .clearValue = wgpu::Color{0.08, 0.08, 0.085, 1.0},
  };
};

} // namespace lab::pipeline_defaults
