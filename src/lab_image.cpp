#include "lab_detail.h"

#include <lab_texture.h>

#include <array>
#include <cstdint>
#include <fstream>
#include <vector>

// A minimal PNG writer, so that saving a screenshot needs no image library.
// The pixel data is stored without compression.

namespace lab {

namespace {

uint32_t crc32(uint32_t crc, const uint8_t* data, size_t size) {
  static const std::array<uint32_t, 256> table = [] {
    std::array<uint32_t, 256> table{};
    for (uint32_t n = 0; n < 256; ++n) {
      uint32_t c = n;
      for (int k = 0; k < 8; ++k) {
        c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
      }
      table[n] = c;
    }
    return table;
  }();
  crc = ~crc;
  for (size_t i = 0; i < size; ++i) {
    crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
  }
  return ~crc;
}

void push_u32(std::vector<uint8_t>& out, uint32_t value) {
  for (int shift = 24; shift >= 0; shift -= 8) {
    out.push_back(static_cast<uint8_t>(value >> shift));
  }
}

void write_chunk(std::ofstream& file, const char type[4], const std::vector<uint8_t>& data) {
  std::vector<uint8_t> chunk;
  push_u32(chunk, static_cast<uint32_t>(data.size()));
  chunk.insert(chunk.end(), type, type + 4);
  chunk.insert(chunk.end(), data.begin(), data.end());
  push_u32(chunk, crc32(0, chunk.data() + 4, chunk.size() - 4)); // the checksum covers type and data
  file.write(reinterpret_cast<const char*>(chunk.data()), static_cast<std::streamsize>(chunk.size()));
}

} // namespace

namespace detail {

void write_png(const std::filesystem::path& path, uint32_t width, uint32_t height, const uint8_t* rgba) {
  std::ofstream file(path, std::ios::binary);
  if (!file) {
    fail("save_png", std::format("could not open \"{}\" for writing", path.string()));
  }
  static constexpr uint8_t signature[] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
  file.write(reinterpret_cast<const char*>(signature), sizeof(signature));

  std::vector<uint8_t> header;
  push_u32(header, width);
  push_u32(header, height);
  header.insert(header.end(), {8, 6, 0, 0, 0}); // 8 bits per channel, RGBA, no interlacing
  write_chunk(file, "IHDR", header);

  // every row is preceded by a filter byte (0: none)
  std::vector<uint8_t> raw;
  raw.reserve((size_t{width} * 4 + 1) * height);
  for (uint32_t y = 0; y < height; ++y) {
    raw.push_back(0);
    raw.insert(raw.end(), rgba + size_t{y} * width * 4, rgba + size_t{y + 1} * width * 4);
  }

  // a zlib stream of "stored" deflate blocks, which hold at most 65535 bytes each
  std::vector<uint8_t> zlib = {0x78, 0x01};
  uint32_t adler_a = 1, adler_b = 0;
  for (size_t offset = 0; offset < raw.size();) {
    const size_t block = std::min<size_t>(65535, raw.size() - offset);
    const bool last = offset + block == raw.size();
    zlib.push_back(last ? 1 : 0);
    zlib.push_back(static_cast<uint8_t>(block & 0xFF));
    zlib.push_back(static_cast<uint8_t>(block >> 8));
    zlib.push_back(static_cast<uint8_t>(~block & 0xFF));
    zlib.push_back(static_cast<uint8_t>((~block >> 8) & 0xFF));
    for (size_t i = offset; i < offset + block; ++i) {
      adler_a = (adler_a + raw[i]) % 65521;
      adler_b = (adler_b + adler_a) % 65521;
    }
    zlib.insert(zlib.end(), raw.begin() + static_cast<std::ptrdiff_t>(offset),
                raw.begin() + static_cast<std::ptrdiff_t>(offset + block));
    offset += block;
  }
  push_u32(zlib, (adler_b << 16) | adler_a);
  write_chunk(file, "IDAT", zlib);
  write_chunk(file, "IEND", {});
}

void save_texture_png(const GpuState& gpu, const wgpu::Texture& texture, const std::filesystem::path& path,
                      std::string_view label) {
  const wgpu::TextureFormat format = texture.GetFormat();
  const bool bgra = format == wgpu::TextureFormat::BGRA8Unorm || format == wgpu::TextureFormat::BGRA8UnormSrgb;
  const bool rgba = format == wgpu::TextureFormat::RGBA8Unorm || format == wgpu::TextureFormat::RGBA8UnormSrgb;
  if (!bgra && !rgba) {
    fail(label, "save_png: only textures with 8-bit RGBA or BGRA formats can be saved");
  }
  const uint32_t width = texture.GetWidth(), height = texture.GetHeight();

  // rows in the buffer a texture is copied to have to start at multiples of 256 bytes
  const uint64_t row_bytes = uint64_t{width} * 4;
  const uint64_t padded_row_bytes = (row_bytes + 255) & ~uint64_t{255};
  wgpu::BufferDescriptor buffer_desc{
      .label = "lab save_png staging buffer",
      .usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst,
      .size = padded_row_bytes * height,
  };
  wgpu::Buffer staging = gpu.device.CreateBuffer(&buffer_desc);

  wgpu::TexelCopyTextureInfo source;
  source.texture = texture;
  wgpu::TexelCopyBufferInfo destination;
  destination.buffer = staging;
  destination.layout.bytesPerRow = static_cast<uint32_t>(padded_row_bytes);
  destination.layout.rowsPerImage = height;
  const wgpu::Extent3D extent{width, height, 1};

  wgpu::CommandEncoder encoder = gpu.device.CreateCommandEncoder();
  encoder.CopyTextureToBuffer(&source, &destination, &extent);
  wgpu::CommandBuffer commands = encoder.Finish();
  gpu.queue.Submit(1, &commands);

  bool mapped = false;
  gpu.wait(staging.MapAsync(
      wgpu::MapMode::Read, 0, wgpu::kWholeMapSize, wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::MapAsyncStatus status, wgpu::StringView, bool* mapped) {
        *mapped = status == wgpu::MapAsyncStatus::Success;
      },
      &mapped));
  if (!mapped) {
    fail(label, "save_png: the texture could not be copied back");
  }

  const auto* bytes = static_cast<const uint8_t*>(staging.GetConstMappedRange());
  std::vector<uint8_t> pixels(row_bytes * height);
  for (uint32_t y = 0; y < height; ++y) {
    const uint8_t* row = bytes + y * padded_row_bytes;
    uint8_t* out = pixels.data() + y * row_bytes;
    for (uint32_t x = 0; x < width; ++x) {
      out[x * 4 + 0] = row[x * 4 + (bgra ? 2 : 0)];
      out[x * 4 + 1] = row[x * 4 + 1];
      out[x * 4 + 2] = row[x * 4 + (bgra ? 0 : 2)];
      out[x * 4 + 3] = row[x * 4 + 3];
    }
  }
  staging.Unmap();

  write_png(path, width, height, pixels.data());
  log(LogLevel::debug, "{}: saved \"{}\"", label, path.string());
}

} // namespace detail

void Texture::save_png(const std::filesystem::path& path) const { detail::save_texture_png(*gpu, texture, path, name); }

} // namespace lab
