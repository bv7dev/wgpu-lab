#ifndef WGPU_LAB_TEXTURE_H
#define WGPU_LAB_TEXTURE_H

#include <lab_buffer.h>
#include <lab_gpu.h>
#include <lab_label.h>
#include <lab_window.h>

#include <filesystem>
#include <span>
#include <vector>

namespace lab {

struct TextureOptions {
  // The default lets a texture be sampled in shaders, rendered into, written and read back
  wgpu::TextureUsage usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment |
                             wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
};

// A 2D image in GPU memory: something to sample in a shader or to render into.
// ```cpp
// lab::Texture texture(gpu, wgpu::TextureFormat::RGBA8Unorm, 256, 256);
// texture.write(pixels);                         // std::vector of 4-byte pixels
// auto group = pipeline.bind_group(0, {{0, texture}});
// ```
class Texture {
public:
  Texture(Gpu& gpu, wgpu::TextureFormat format, uint32_t width, uint32_t height, TextureOptions options = {},
          Label label = {});

  Texture(Texture&&) = default;
  Texture& operator=(Texture&&) = default;

  // Replaces all pixels. P is the type of one pixel and has to match the format
  // in size, e.g. a struct of four uint8_t for RGBA8Unorm.
  template<GpuData P>
  void write(std::span<const P> pixels) {
    write_bytes(pixels.data(), pixels.size_bytes(), sizeof(P));
  }
  template<GpuData P>
  void write(const std::vector<P>& pixels) {
    write(std::span<const P>{pixels});
  }

  // Copies all pixels back from the GPU, row by row from the top left
  //  - blocks until the GPU has caught up
  template<GpuData P>
  std::vector<P> read() const {
    std::vector<P> pixels(static_cast<size_t>(extent.width) * extent.height);
    read_bytes(pixels.data(), pixels.size() * sizeof(P), sizeof(P));
    return pixels;
  }

  // Writes the texture to an image file (for 8-bit RGBA and BGRA formats)
  //  - blocks until the GPU has caught up
  void save_png(const std::filesystem::path& path) const;

  // a view of the whole texture, for bind groups and render passes
  wgpu::TextureView view() const;

  wgpu::TextureFormat format() const { return pixel_format; }
  Size size() const { return {static_cast<int>(extent.width), static_cast<int>(extent.height)}; }
  float aspect() const { return static_cast<float>(extent.width) / static_cast<float>(extent.height); }

  const wgpu::Texture& handle() const { return texture; }
  const std::string& label() const { return name; }

private:
  void write_bytes(const void* data, uint64_t byte_count, uint64_t pixel_size);
  void read_bytes(void* out, uint64_t byte_count, uint64_t pixel_size) const;

  std::shared_ptr<detail::GpuState> gpu;
  wgpu::Texture texture;
  wgpu::TextureFormat pixel_format;
  wgpu::Extent3D extent;
  std::string name;
};

} // namespace lab

#endif // WGPU_LAB_TEXTURE_H
