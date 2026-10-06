#ifndef WGPU_LAB_WINDOW_H
#define WGPU_LAB_WINDOW_H

#include <lab_input.h>

#include <functional>
#include <memory>
#include <string_view>

struct GLFWwindow;

namespace lab {

namespace detail {
struct WindowState;
}

struct Size {
  int width = 0, height = 0;
  bool operator==(const Size&) const = default;
};

struct Vec2 {
  float x = 0.0f, y = 0.0f;
};

struct WindowOptions {
  bool resizable = true;
  bool visible = true;
};

// A window on the desktop, with its keyboard and mouse input.
// A window knows nothing about the GPU, a `lab::Surface` connects the two.
// ```cpp
// lab::Window window("Hello", 640, 400);
// while (lab::tick()) {
//   if (window.key_pressed(lab::KeyCode::escape)) window.close();
// }
// ```
class Window {
public:
  // Throws lab::Error if the window cannot be created (e.g. there is no display)
  Window(std::string_view title, int width, int height, WindowOptions options = {});

  Window(Window&&) = default;
  Window& operator=(Window&&) = default;

  // `tick()` returns true as long as at least one window is open
  bool is_open() const;
  void close();

  void set_title(std::string_view title);

  // size in screen coordinates, which is what the window manager works with
  Size size() const;
  // size in pixels, which is what gets rendered: larger than `size()` on scaled displays
  Size framebuffer_size() const;
  // width divided by height
  float aspect() const;

  // Keyboard and mouse state, updated once per `tick()` ---------------------------

  bool key(KeyCode key) const;          // is held down
  bool key_pressed(KeyCode key) const;  // went down since the previous tick
  bool key_released(KeyCode key) const; // went up since the previous tick

  bool mouse_button(MouseButton button) const;
  bool mouse_pressed(MouseButton button) const;
  bool mouse_released(MouseButton button) const;

  Vec2 mouse() const;       // cursor position in pixels, from the top left corner
  Vec2 mouse_delta() const; // cursor movement since the previous tick
  Vec2 scroll() const;      // scroll wheel movement since the previous tick

  // Callbacks, called from within `tick()` as the events arrive -------------------
  // (pass nullptr to remove one)

  void on_key(std::function<void(const KeyEvent&)> callback);
  void on_mouse_button(std::function<void(const MouseButtonEvent&)> callback);
  void on_scroll(std::function<void(Vec2 offset)> callback);
  void on_resize(std::function<void(Size framebuffer_size)> callback);

  GLFWwindow* handle() const;

  // internal: shared with the surfaces of this window
  const std::shared_ptr<detail::WindowState>& state() const { return shared_state; }

private:
  std::shared_ptr<detail::WindowState> shared_state;
};

} // namespace lab

#endif // WGPU_LAB_WINDOW_H
