#ifndef WGPU_LAB_WINDOW_H
#define WGPU_LAB_WINDOW_H

#include <functional>

namespace lab {

using Handle = void*;

struct Window {
  using KeyCallback = std::function<void(int key, int scancode, int action, int mod)>;

  Window(int width, int height, const char* title);

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  void set_key_callback(KeyCallback cb);
  void clear_key_callback();

  int width() const;
  int height() const;

  Handle get_handle() const;

  bool is_open() const;

  ~Window();

private:
  KeyCallback keycb;
  Handle handle;
};

} // namespace lab

#endif // WGPU_LAB_WINDOW_H
