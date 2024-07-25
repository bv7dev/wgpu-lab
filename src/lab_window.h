#ifndef WGPU_LAB_WINDOW_H
#define WGPU_LAB_WINDOW_H

#include <functional>

#include "lab_webgpu.h"

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

  void init_surface(Webgpu&);

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

#endif // WGPU_LAB_WINDOW_H
