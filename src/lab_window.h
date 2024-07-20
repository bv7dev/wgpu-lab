#ifndef LAB_WINDOW_H
#define LAB_WINDOW_H

#include <functional>

namespace lab {

using KeyCallback = std::function<void(int key, int scancode, int action, int mod)>;
using WindowHandle = void*;

class Window {
  WindowHandle _window;
  KeyCallback _key_callback;

public:
  Window(int width, int height, const char* title);

  void set_callback(KeyCallback cb);
  void clear_callback();

  bool is_open();

  ~Window();
};

bool tick();

} // namespace lab

#endif // LAB_WINDOW_H
