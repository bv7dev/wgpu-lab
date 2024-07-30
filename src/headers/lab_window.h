#ifndef WGPU_LAB_WINDOW_H
#define WGPU_LAB_WINDOW_H

#include <functional>

#include "lab_enums.h"

struct GLFWwindow;
using GlfwWindowHandle = GLFWwindow*;

namespace lab {

struct KeyEvent {
  KeyCode key = KeyCode::unknown;
  InputAction action = InputAction::none;
  ModKey mod = ModKey::none;
  int scancode = 0;
};

bool operator==(const KeyEvent& lhs, const KeyEvent& rhs);

struct Window {
  using KeyCallback = std::function<void(const KeyEvent&)>;
  using ResizeCallback = std::function<void(int width, int height)>;

  Window(const char* title, int width, int height);

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  void set_key_callback(KeyCallback);
  void clear_key_callback();

  void set_resize_callback(ResizeCallback);
  void clear_resize_callback();

  void set_title(const char* title);

  int width() const;
  int height() const;

  GlfwWindowHandle get_handle() const;

  bool is_open() const;

  ~Window();

private:
  KeyCallback user_key_callback;
  ResizeCallback user_resize_callback;
  GlfwWindowHandle handle;
};

using WindowHandle = Window*;

} // namespace lab

#endif // WGPU_LAB_WINDOW_H
