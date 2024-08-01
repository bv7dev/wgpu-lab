#include <webgpu/webgpu.hpp>

namespace lab::PipelineDefaults {

struct InitConfig {
  wgpu::BlendState blendState = {{
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
  }};

  wgpu::ColorTargetState colorTarget = {{
      .writeMask = wgpu::ColorWriteMask::All,
  }};

  wgpu::FragmentState fragmentState = {{
      .entryPoint = "fs_main",
  }};

  wgpu::VertexState vertexState = {{
      .entryPoint = "vs_main",
  }};

  wgpu::PrimitiveState primitiveState = {{
      .topology = wgpu::PrimitiveTopology::TriangleList,
      .stripIndexFormat = wgpu::IndexFormat::Undefined,
      .frontFace = wgpu::FrontFace::CCW,
      .cullMode = wgpu::CullMode::None,
  }};

  wgpu::MultisampleState multisampleState = {{
      .count = 1,
      .mask = ~0u,
  }};

  wgpu::VertexBufferLayout vertexBufferLayout{};
  wgpu::VertexAttribute vertexAttribute_0{};
};

struct RenderConfig {
  wgpu::RenderPassColorAttachment renderPassColorAttachment = {{
      .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
      .loadOp = wgpu::LoadOp::Clear,
      .storeOp = wgpu::StoreOp::Store,
      .clearValue = WGPUColor{0.08, 0.08, 0.085, 1.0},
  }};

  wgpu::RenderPassDescriptor renderPassDesc = {{
      .label = "Default Render Pass",
      .colorAttachmentCount = 1,
  }};
};

} // namespace lab::PipelineDefaults
