#ifndef WGPU_LAB_LABEL_H
#define WGPU_LAB_LABEL_H

#include <source_location>
#include <string>
#include <string_view>

namespace lab {

// The name of a GPU object, it shows up in error messages of the lab and of WebGPU.
// An object that is given no name is named after the place where it is created:
// ```cpp
// lab::Buffer vertices(gpu, vertex_data);                                 // "buffer main.cpp:23"
// lab::Buffer vertices(gpu, vertex_data, wgpu::BufferUsage::Vertex, "triangle"); // "triangle"
// ```
struct Label {
  std::string text;

  Label(std::source_location where = std::source_location::current());
  Label(const char* text) : text{text} {}
  Label(std::string_view text) : text{text} {}
  Label(std::string text) : text{std::move(text)} {}

  // prefixes a label that was derived from the source location with the kind of object
  Label&& as(std::string_view kind) &&;

private:
  bool from_location = false;
};

} // namespace lab

#endif // WGPU_LAB_LABEL_H
