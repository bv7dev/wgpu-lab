#ifndef LAB_WINDOW_H
#define LAB_WINDOW_H

#include <functional>

namespace lab {

using KeyCallback = std::function<void(int key, int scancode, int action, int mod)>;
using WindowHandle = void*;

class Window {
  WindowHandle _handle;
  KeyCallback _key_callback;

public:
  Window(int width, int height, const char* title);

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  void set_key_callback(KeyCallback cb);
  void clear_key_callback();

  bool is_open();

  ~Window();
};

bool tick();

} // namespace lab

#endif // LAB_WINDOW_H
