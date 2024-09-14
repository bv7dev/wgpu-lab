#ifndef WGPU_LAB_WINDOW_H
#define WGPU_LAB_WINDOW_H

#include <extra/lab_enums.h>
#include <extra/lab_objects.h>

#include <functional>

namespace lab {

struct Window {
  Window(const char* title, int width, int height);

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  void set_key_callback(std::function<void(const KeyEvent&)>);
  void clear_key_callback();

  void set_resize_callback(std::function<void(int width, int height)>);
  void clear_resize_callback();

  void set_title(const char* title);

  int width() const;
  int height() const;
  float ratio() const;

  bool is_open() const;
  void close();
  ~Window();

  std::function<void(int width, int height)> user_resize_callback;
  std::function<void(const KeyEvent&)> user_key_callback;

  GLFWwindowHandle glfw_window_handle;
};

} // namespace lab

#endif // WGPU_LAB_WINDOW_H
