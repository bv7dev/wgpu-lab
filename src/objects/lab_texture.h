#ifndef WGPU_LAB_TEXTURE_H
#define WGPU_LAB_TEXTURE_H

#include <objects/lab_webgpu.h>

namespace lab {

// WIP todos:
// - allow multi-threaded (read/write) mapping like lab::Buffer (reuse MappedVRAM)
// - support 3d textures
// - support texture arrays
struct Texture {
  Webgpu& webgpu;

  wgpu::TextureDescriptor descriptor = {
      .label = "lab default texture",
      .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
      .mipLevelCount = 1,
      .sampleCount = 1,
      .viewFormatCount = 0,
      .viewFormats = nullptr,
  };

  Texture(Webgpu& instance, wgpu::TextureFormat format, uint32_t width, uint32_t height = 1,
          uint32_t depthOrArrayLayers = 1)
      : webgpu{instance} {
    if (height > 1 && depthOrArrayLayers > 1) {
      descriptor.dimension = wgpu::TextureDimension::e3D;
    } else if (height > 1 && depthOrArrayLayers == 1) {
      descriptor.dimension = wgpu::TextureDimension::e2D;
    } else if (height == 1 && depthOrArrayLayers == 1) {
      descriptor.dimension = wgpu::TextureDimension::e1D;
    }
    descriptor.size = {width, height, depthOrArrayLayers};
    descriptor.format = format;
  }

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  [[nodiscard]] wgpu::Texture transfer() { return webgpu.device.CreateTexture(&descriptor); }

  wgpu::ImageCopyTexture target = {
      .mipLevel = 0,
      .origin = {0, 0, 0},
      .aspect = wgpu::TextureAspect::All,
  };

  wgpu::Texture wgpu_texture = nullptr;

  template<typename T>
  void to_device(const std::vector<T>& pixels) {
    wgpu_texture = transfer();
    target.texture = wgpu_texture;

    wgpu::TextureDataLayout layout = {
        .offset = 0,
        .bytesPerRow = sizeof(T) * descriptor.size.width,
        .rowsPerImage = descriptor.size.height,
    };

    webgpu.queue.WriteTexture(&target, pixels.data(), pixels.size() * sizeof(T), &layout,
                              &descriptor.size);
  }

  inline int width() const noexcept { return static_cast<int>(descriptor.size.width); }
  inline int height() const noexcept { return static_cast<int>(descriptor.size.height); }

  mutable wgpu::TextureViewDescriptor textureViewDesc = {
      .label = "lab default texture view",
      .dimension = wgpu::TextureViewDimension::e2D,
      .baseMipLevel = 0,
      .mipLevelCount = 1,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .aspect = wgpu::TextureAspect::All,
  };

  [[nodiscard]] wgpu::TextureView create_view() const {
    assert(wgpu_texture != nullptr);
    textureViewDesc.format = descriptor.format;
    return wgpu_texture.CreateView(&textureViewDesc);
  }

  ~Texture() {
    if (wgpu_texture) {
      wgpu_texture.Destroy();
    }
  }
};

} // namespace lab

#endif // WGPU_LAB_TEXTURE_H
