#ifndef WGPU_LAB_TESTS_OFFSCREEN_H
#define WGPU_LAB_TESTS_OFFSCREEN_H

#include <doctest/doctest.h>

#include <lab>

#include <cstdint>
#include <vector>

// Renders with a lab::Pipeline into a texture instead of a window and returns the pixels.
// This is written against raw WebGPU because the lab cannot render offscreen by itself yet.

namespace offscreen {

struct Pixel {
  uint8_t r, g, b, a;
  bool operator==(const Pixel&) const = default;
};

inline doctest::String toString(const Pixel& p) {
  return doctest::String(std::format("({}, {}, {}, {})", p.r, p.g, p.b, p.a).c_str());
}

// the render target is a square of this many pixels, cleared to opaque red
inline constexpr uint32_t size = 64;
inline constexpr wgpu::TextureFormat format = wgpu::TextureFormat::RGBA8Unorm;
inline constexpr Pixel clear_color{255, 0, 0, 255};

inline constexpr size_t center = (size / 2) * size + size / 2; // index of the pixel in the middle
inline constexpr size_t corner = 0;                            // index of the top left pixel

// Draws `count` vertices (or indices, if the pipeline has an index buffer) and returns size * size pixels.
// The pipeline has to be finalized with `webgpu.surface_format = offscreen::format`.
inline std::vector<Pixel> render(lab::Pipeline& pipeline, uint32_t count, uint32_t instances = 1) {
  wgpu::Device device = pipeline.webgpu.device;

  wgpu::TextureDescriptor textureDesc{
      .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
      .size = {size, size, 1},
      .format = format,
  };
  wgpu::Texture texture = device.CreateTexture(&textureDesc);

  wgpu::RenderPassColorAttachment attachment{
      .view = texture.CreateView(),
      .loadOp = wgpu::LoadOp::Clear,
      .storeOp = wgpu::StoreOp::Store,
      .clearValue = {1.0, 0.0, 0.0, 1.0},
  };
  wgpu::RenderPassDescriptor passDesc{.colorAttachmentCount = 1, .colorAttachments = &attachment};

  wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
  wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);
  pass.SetPipeline(pipeline.wgpu_pipeline);
  for (uint32_t i = 0; i < pipeline.vb_configs.size(); ++i) {
    pass.SetVertexBuffer(i, pipeline.vb_configs[i].buffer, pipeline.vb_configs[i].offset);
  }
  for (uint32_t i = 0; i < pipeline.bindGroups.size(); ++i) {
    pass.SetBindGroup(i, pipeline.bindGroups[i]);
  }
  if (pipeline.ib_configs.empty()) {
    pass.Draw(count, instances);
  } else {
    const auto& index_buffer = pipeline.ib_configs.front();
    pass.SetIndexBuffer(index_buffer.buffer, index_buffer.format, index_buffer.offset);
    pass.DrawIndexed(count, instances);
  }
  pass.End();

  // copy the texture into a buffer that can be mapped and read
  wgpu::BufferDescriptor readbackDesc{
      .usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst,
      .size = size * size * sizeof(Pixel),
  };
  wgpu::Buffer readback = device.CreateBuffer(&readbackDesc);

  wgpu::TexelCopyTextureInfo source;
  source.texture = texture;
  wgpu::TexelCopyBufferInfo destination;
  destination.buffer = readback;
  destination.layout.bytesPerRow = size * sizeof(Pixel); // 256, which is the required row alignment
  destination.layout.rowsPerImage = size;
  wgpu::Extent3D extent{size, size, 1};
  encoder.CopyTextureToBuffer(&source, &destination, &extent);

  wgpu::CommandBuffer commands = encoder.Finish();
  pipeline.webgpu.queue.Submit(1, &commands);

  bool mapped = false;
  wgpu::Future future = readback.MapAsync(
      wgpu::MapMode::Read, 0, wgpu::kWholeMapSize, wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::MapAsyncStatus status, wgpu::StringView, bool* mapped) {
        *mapped = status == wgpu::MapAsyncStatus::Success;
      },
      &mapped);
  pipeline.webgpu.instance.WaitAny(future, UINT64_MAX);
  REQUIRE(mapped);

  const Pixel* data = static_cast<const Pixel*>(readback.GetConstMappedRange());
  std::vector<Pixel> pixels(data, data + size * size);
  readback.Unmap();
  return pixels;
}

} // namespace offscreen

#endif // WGPU_LAB_TESTS_OFFSCREEN_H
