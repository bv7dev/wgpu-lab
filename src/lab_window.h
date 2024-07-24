#ifndef LAB_WINDOW_H
#define LAB_WINDOW_H

#include <functional>

namespace lab {

using WindowHandle = void*;

class Window {
  using KeyCallback = std::function<void(int key, int scancode, int action, int mod)>;
  struct Dimensions {
    int width;
    int height;
  };

  WindowHandle _handle;
  KeyCallback _key_callback;

public:
  Window(int width, int height, const char* title);

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  void set_key_callback(KeyCallback cb);
  void clear_key_callback();

  Dimensions get_dimensions() const;
  WindowHandle get_handle() const;

  bool is_open() const;

  ~Window();
};

bool tick();

} // namespace lab

#endif // LAB_WINDOW_H
