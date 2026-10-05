#include "lab_detail.h"

#include <lab_texture.h>

#include <webgpu/webgpu_cpp_print.h>

#include <cstring>
#include <sstream>
#include <vector>

namespace lab {

namespace {

// Bytes per pixel of the formats that write() and read() support, 0 for all others
uint64_t pixel_size(wgpu::TextureFormat format) {
  using enum wgpu::TextureFormat;
  switch (format) {
  case R8Unorm:
  case R8Snorm:
  case R8Uint:
  case R8Sint:
    return 1;
  case R16Uint:
  case R16Sint:
  case R16Float:
  case RG8Unorm:
  case RG8Snorm:
  case RG8Uint:
  case RG8Sint:
    return 2;
  case R32Float:
  case R32Uint:
  case R32Sint:
  case RG16Uint:
  case RG16Sint:
  case RG16Float:
  case RGBA8Unorm:
  case RGBA8UnormSrgb:
  case RGBA8Snorm:
  case RGBA8Uint:
  case RGBA8Sint:
  case BGRA8Unorm:
  case BGRA8UnormSrgb:
  case RGB10A2Unorm:
  case RGB10A2Uint:
  case RG11B10Ufloat:
    return 4;
  case RG32Float:
  case RG32Uint:
  case RG32Sint:
  case RGBA16Uint:
  case RGBA16Sint:
  case RGBA16Float:
    return 8;
  case RGBA32Float:
  case RGBA32Uint:
  case RGBA32Sint:
    return 16;
  default:
    return 0;
  }
}

std::string format_name(wgpu::TextureFormat format) {
  std::ostringstream stream;
  stream << format;
  return stream.str();
}

// throws unless `pixel_count` pixels of `given_size` bytes are exactly the content of the texture
void check_pixels(const std::string& label, const char* what, wgpu::TextureFormat format, wgpu::Extent3D extent,
                  uint64_t byte_count, uint64_t given_size) {
  const uint64_t expected_size = pixel_size(format);
  if (expected_size == 0) {
    detail::fail(label, std::format("{}: not supported for textures of format {}", what, format_name(format)));
  }
  if (given_size != expected_size) {
    detail::fail(label, std::format("{}: a pixel of format {} has {} bytes, but the pixel type has {}", what,
                                    format_name(format), expected_size, given_size));
  }
  const uint64_t expected_bytes = uint64_t{extent.width} * extent.height * expected_size;
  if (byte_count != expected_bytes) {
    detail::fail(label, std::format("{}: the texture has {}x{} = {} pixels, but {} were given", what, extent.width,
                                    extent.height, uint64_t{extent.width} * extent.height, byte_count / given_size));
  }
}

} // namespace

Texture::Texture(Gpu& gpu_object, wgpu::TextureFormat format, uint32_t width, uint32_t height, TextureOptions options,
                 Label label)
    : gpu{gpu_object.state()}, pixel_format{format}, extent{width, height, 1},
      name{std::move(label).as("texture").text} {
  wgpu::TextureDescriptor desc{
      .label = std::string_view(name),
      .usage = options.usage,
      .dimension = wgpu::TextureDimension::e2D,
      .size = extent,
      .format = format,
  };
  if (auto error = detail::capture_error(*gpu, [&] { texture = gpu->device.CreateTexture(&desc); })) {
    detail::fail(name, *error);
  }
}

wgpu::TextureView Texture::view() const { return texture.CreateView(); }

wgpu::Sampler sampler(Gpu& gpu, SamplerOptions options) {
  wgpu::SamplerDescriptor desc;
  desc.label = std::string_view(options.label);
  desc.addressModeU = options.address_mode;
  desc.addressModeV = options.address_mode;
  desc.addressModeW = options.address_mode;
  desc.magFilter = options.filter;
  desc.minFilter = options.filter;
  return gpu.device().CreateSampler(&desc);
}

void Texture::write_bytes(const void* data, uint64_t byte_count, uint64_t given_pixel_size) {
  check_pixels(name, "write", pixel_format, extent, byte_count, given_pixel_size);

  wgpu::TexelCopyTextureInfo destination;
  destination.texture = texture;
  wgpu::TexelCopyBufferLayout layout{
      .bytesPerRow = static_cast<uint32_t>(extent.width * given_pixel_size),
      .rowsPerImage = extent.height,
  };
  gpu->queue.WriteTexture(&destination, data, byte_count, &layout, &extent);
}

void Texture::read_bytes(void* out, uint64_t byte_count, uint64_t given_pixel_size) const {
  check_pixels(name, "read", pixel_format, extent, byte_count, given_pixel_size);

  // rows in the buffer a texture is copied to have to start at multiples of 256 bytes
  const uint64_t row_bytes = extent.width * given_pixel_size;
  const uint64_t padded_row_bytes = (row_bytes + 255) & ~uint64_t{255};

  wgpu::BufferDescriptor buffer_desc{
      .label = "lab texture read staging buffer",
      .usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst,
      .size = padded_row_bytes * extent.height,
  };
  wgpu::Buffer staging = gpu->device.CreateBuffer(&buffer_desc);

  wgpu::TexelCopyTextureInfo source;
  source.texture = texture;
  wgpu::TexelCopyBufferInfo destination;
  destination.buffer = staging;
  destination.layout.bytesPerRow = static_cast<uint32_t>(padded_row_bytes);
  destination.layout.rowsPerImage = extent.height;

  wgpu::CommandEncoder encoder = gpu->device.CreateCommandEncoder();
  encoder.CopyTextureToBuffer(&source, &destination, &extent);
  wgpu::CommandBuffer commands = encoder.Finish();
  gpu->queue.Submit(1, &commands);

  bool mapped = false;
  gpu->wait(staging.MapAsync(
      wgpu::MapMode::Read, 0, wgpu::kWholeMapSize, wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::MapAsyncStatus status, wgpu::StringView, bool* mapped) {
        *mapped = status == wgpu::MapAsyncStatus::Success;
      },
      &mapped));
  if (!mapped) {
    detail::fail(name, "read: the texture could not be copied back");
  }

  const auto* bytes = static_cast<const char*>(staging.GetConstMappedRange());
  for (uint32_t row = 0; row < extent.height; ++row) {
    std::memcpy(static_cast<char*>(out) + row * row_bytes, bytes + row * padded_row_bytes, row_bytes);
  }
  staging.Unmap();
}

} // namespace lab
