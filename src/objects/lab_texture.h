#ifndef WGPU_LAB_TEXTURE_H
#define WGPU_LAB_TEXTURE_H

#include <objects/lab_webgpu.h>

namespace lab {

struct Texture {
  Webgpu& webgpu;

  WGPUTextureDescriptor descriptor = {
      .label = "lab default texture",
      .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
      .size = {0, 0, 0},
      .format = wgpu::TextureFormat::RGBA8Unorm,
      .mipLevelCount = 1,
      .sampleCount = 1,
      .viewFormatCount = 0,
      .viewFormats = nullptr,
  };

  Texture(WGPUExtent2D size, Webgpu& instance) : webgpu{instance} {
    descriptor.dimension = wgpu::TextureDimension::_2D;
    descriptor.size = {size.width, size.height, 1};
  }

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  [[nodiscard]] wgpu::Texture transfer() { return webgpu.device.createTexture(descriptor); }

  WGPUImageCopyTexture target = {
      .mipLevel = 0,
      .origin = {0, 0, 0},
      .aspect = wgpu::TextureAspect::All,
  };

  wgpu::Texture wgpu_texture = nullptr;

  void to_device(const std::vector<uint8_t>& pixels) {
    wgpu_texture = transfer();
    target.texture = wgpu_texture;

    WGPUTextureDataLayout layout = {
        .offset = 0,
        .bytesPerRow = 4 * descriptor.size.width,
        .rowsPerImage = descriptor.size.height,
    };

    webgpu.queue.writeTexture(target, pixels.data(), pixels.size(), layout, descriptor.size);
  }

  inline uint32_t width() const noexcept { return descriptor.size.width; }
  inline uint32_t height() const noexcept { return descriptor.size.height; }

  WGPUTextureViewDescriptor textureViewDesc = {
      .label = "lab default texture view",
      .dimension = wgpu::TextureViewDimension::_2D,
      .baseMipLevel = 0,
      .mipLevelCount = 1,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .aspect = wgpu::TextureAspect::All,
  };

  [[nodiscard]] wgpu::TextureView create_view() {
    assert(wgpu_texture != nullptr);
    textureViewDesc.format = descriptor.format;
    return wgpu_texture.createView(textureViewDesc);
  }

  ~Texture() {
    if (wgpu_texture) {
      wgpu_texture.destroy();
      wgpu_texture.release();
    }
  }
};

} // namespace lab

#endif // WGPU_LAB_TEXTURE_H
