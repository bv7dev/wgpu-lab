#ifndef LAB_WINDOW_H
#define LAB_WINDOW_H

#include <functional>

#include <webgpu/webgpu.hpp>

#include "lab_state.h"

namespace lab {

struct Window {
  using KeyCallback = std::function<void(int key, int scancode, int action, int mod)>;

  struct Dimensions {
    int width;
    int height;
  };

  Window(int width, int height, const char* title);

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  void set_key_callback(KeyCallback cb);
  void clear_key_callback();

  Dimensions get_dimensions() const;
  Handle get_handle() const;

  bool is_open() const;

  ~Window();

  wgpu::Surface surface = nullptr;

private:
  KeyCallback keycb;
  Handle handle;
  State& state;
};

} // namespace lab

#endif // LAB_WINDOW_H
