#ifndef WGPU_LAB_TESTS_COMMON_H
#define WGPU_LAB_TESTS_COMMON_H

#include <doctest/doctest.h>

#include <lab>

#include <cstdint>
#include <format>
#include <string>
#include <vector>

// One RGBA8 pixel as read back from a texture
struct Pixel {
  uint8_t r, g, b, a;
  bool operator==(const Pixel&) const = default;
};

inline doctest::String toString(const Pixel& p) {
  return doctest::String(std::format("({}, {}, {}, {})", p.r, p.g, p.b, p.a).c_str());
}

// The render tests draw into a square texture of this many pixels, cleared to opaque red
inline constexpr uint32_t target_size = 64;
inline constexpr wgpu::TextureFormat target_format = wgpu::TextureFormat::RGBA8Unorm;
inline constexpr wgpu::Color clear_color{1.0, 0.0, 0.0, 1.0};
inline constexpr Pixel clear_pixel{255, 0, 0, 255};

// index of the pixel at (x, y), counted from the top left
inline constexpr size_t at(uint32_t x, uint32_t y) { return y * target_size + x; }
inline constexpr size_t center = at(target_size / 2, target_size / 2);
inline constexpr size_t corner = at(0, 0);

inline lab::Texture make_target(lab::Gpu& gpu) { return lab::Texture(gpu, target_format, target_size, target_size); }

// a vertex shader that covers the whole target with a single triangle and needs no buffers
inline const std::string fullscreen_vertex_shader = R"(
  @vertex fn vs_main(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {
    var positions = array<vec2f, 3>(vec2f(-1.0, -1.0), vec2f(3.0, -1.0), vec2f(-1.0, 3.0));
    return vec4f(positions[i], 0.0, 1.0);
  }
)";

#endif // WGPU_LAB_TESTS_COMMON_H
