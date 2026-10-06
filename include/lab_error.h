#ifndef WGPU_LAB_ERROR_H
#define WGPU_LAB_ERROR_H

#include <stdexcept>
#include <string_view>

namespace lab {

// Thrown when the lab detects a mistake or cannot go on: a file that does not
// exist, a shader that does not compile, a vertex buffer that does not fit the
// pipeline it is drawn with, an error reported by the GPU, ...
//  - `what()` starts with the label of the object the error is about
struct Error : std::runtime_error {
  using std::runtime_error::runtime_error;
};

namespace detail {

// Logs the message as an error and throws lab::Error("<object>: <message>")
[[noreturn]] void fail(std::string_view object, std::string_view message);

} // namespace detail

} // namespace lab

#endif // WGPU_LAB_ERROR_H
