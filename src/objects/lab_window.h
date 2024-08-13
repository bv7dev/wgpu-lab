#ifndef WGPU_LAB_WINDOW_H
#define WGPU_LAB_WINDOW_H

#include <extra/lab_enums.h>
#include <extra/lab_objects.h>

#include <functional>

namespace lab {

struct KeyEvent {
  KeyCode key = KeyCode::unknown;
  InputType action = InputType::none;
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
  float ratio() const { return (1.f / width()) * height(); }

  GlfwWindowHandle get_handle() const;

  bool is_open() const;

  void close();
  ~Window();

private:
  KeyCallback user_key_callback;
  ResizeCallback user_resize_callback;
  GlfwWindowHandle handle;
};

} // namespace lab

#endif // WGPU_LAB_WINDOW_H
