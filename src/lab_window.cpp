#include "lab_detail.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <string>

namespace lab {

namespace detail {

namespace {

WindowState& state_of(GLFWwindow* window) { return *static_cast<WindowState*>(glfwGetWindowUserPointer(window)); }

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  WindowState& state = state_of(window);
  if (key >= 0 && static_cast<size_t>(key) < WindowState::key_count) {
    if (action == GLFW_PRESS) {
      state.keys_down.set(key);
      state.keys_pressed.set(key);
    } else if (action == GLFW_RELEASE) {
      state.keys_down.reset(key);
      state.keys_released.set(key);
    }
  }
  if (state.on_key) {
    state.on_key({static_cast<KeyCode>(key), static_cast<KeyAction>(action), static_cast<ModKey>(mods), scancode});
  }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  WindowState& state = state_of(window);
  if (button >= 0 && static_cast<size_t>(button) < WindowState::button_count) {
    if (action == GLFW_PRESS) {
      state.buttons_down.set(button);
      state.buttons_pressed.set(button);
    } else if (action == GLFW_RELEASE) {
      state.buttons_down.reset(button);
      state.buttons_released.set(button);
    }
  }
  if (state.on_mouse_button) {
    state.on_mouse_button(
        {static_cast<MouseButton>(button), static_cast<KeyAction>(action), static_cast<ModKey>(mods)});
  }
}

void cursor_callback(GLFWwindow* window, double x, double y) {
  WindowState& state = state_of(window);

  // GLFW reports the cursor in screen coordinates, the lab works in pixels
  int window_width, window_height, pixel_width, pixel_height;
  glfwGetWindowSize(window, &window_width, &window_height);
  glfwGetFramebufferSize(window, &pixel_width, &pixel_height);
  const float scale_x = window_width > 0 ? static_cast<float>(pixel_width) / static_cast<float>(window_width) : 1.0f;
  const float scale_y = window_height > 0 ? static_cast<float>(pixel_height) / static_cast<float>(window_height) : 1.0f;
  const Vec2 position{static_cast<float>(x) * scale_x, static_cast<float>(y) * scale_y};

  if (state.mouse_known) {
    state.mouse_delta.x += position.x - state.mouse.x;
    state.mouse_delta.y += position.y - state.mouse.y;
  }
  state.mouse = position;
  state.mouse_known = true;
}

void scroll_callback(GLFWwindow* window, double x, double y) {
  WindowState& state = state_of(window);
  const Vec2 offset{static_cast<float>(x), static_cast<float>(y)};
  state.scroll.x += offset.x;
  state.scroll.y += offset.y;
  if (state.on_scroll) {
    state.on_scroll(offset);
  }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  WindowState& state = state_of(window);
  if (state.on_resize) {
    state.on_resize({width, height});
  }
}

} // namespace

void WindowState::begin_tick() {
  keys_pressed.reset();
  keys_released.reset();
  buttons_pressed.reset();
  buttons_released.reset();
  mouse_delta = {};
  scroll = {};
}

void WindowState::close() {
  if (!open) {
    return;
  }
  open = false;
  glfwHideWindow(handle);
  std::erase(Runtime::get().windows, this);
  log(LogLevel::debug, "window \"{}\" closed", glfwGetWindowTitle(handle));
}

WindowState::~WindowState() {
  if (!handle) {
    return;
  }
  close();
  // no callback may reach this state anymore
  glfwSetKeyCallback(handle, nullptr);
  glfwSetMouseButtonCallback(handle, nullptr);
  glfwSetCursorPosCallback(handle, nullptr);
  glfwSetScrollCallback(handle, nullptr);
  glfwSetFramebufferSizeCallback(handle, nullptr);
  glfwSetWindowUserPointer(handle, nullptr);
  Runtime::get().retire_window(handle);
}

} // namespace detail

Window::Window(std::string_view title, int width, int height, WindowOptions options)
    : shared_state{std::make_shared<detail::WindowState>()} {
  detail::Runtime& runtime = detail::Runtime::get();
  runtime.init_glfw();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // WebGPU renders, not OpenGL
  glfwWindowHint(GLFW_RESIZABLE, options.resizable ? GLFW_TRUE : GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, options.visible ? GLFW_TRUE : GLFW_FALSE);

  const std::string title_string{title};
  GLFWwindow* handle = glfwCreateWindow(width, height, title_string.c_str(), nullptr, nullptr);
  if (!handle) {
    detail::fail(std::format("window \"{}\"", title), "could not be created");
  }

  detail::WindowState& state = *shared_state;
  state.handle = handle;
  state.open = true;
  glfwSetWindowUserPointer(handle, &state);
  glfwSetKeyCallback(handle, detail::key_callback);
  glfwSetMouseButtonCallback(handle, detail::mouse_button_callback);
  glfwSetCursorPosCallback(handle, detail::cursor_callback);
  glfwSetScrollCallback(handle, detail::scroll_callback);
  glfwSetFramebufferSizeCallback(handle, detail::framebuffer_size_callback);

  runtime.windows.push_back(&state);
  detail::log(LogLevel::debug, "window \"{}\" created", title);
}

bool Window::is_open() const { return shared_state->open; }
void Window::close() { shared_state->close(); }

void Window::set_title(std::string_view title) { glfwSetWindowTitle(shared_state->handle, std::string{title}.c_str()); }

Size Window::size() const {
  Size size;
  glfwGetWindowSize(shared_state->handle, &size.width, &size.height);
  return size;
}

Size Window::framebuffer_size() const {
  Size size;
  glfwGetFramebufferSize(shared_state->handle, &size.width, &size.height);
  return size;
}

float Window::aspect() const {
  const Size size = framebuffer_size();
  return size.height > 0 ? static_cast<float>(size.width) / static_cast<float>(size.height) : 1.0f;
}

namespace {

template<size_t N>
bool test(const std::bitset<N>& bits, int index) {
  return index >= 0 && static_cast<size_t>(index) < N && bits.test(static_cast<size_t>(index));
}

} // namespace

bool Window::key(KeyCode key) const { return test(shared_state->keys_down, static_cast<int>(key)); }
bool Window::key_pressed(KeyCode key) const { return test(shared_state->keys_pressed, static_cast<int>(key)); }
bool Window::key_released(KeyCode key) const { return test(shared_state->keys_released, static_cast<int>(key)); }

bool Window::mouse_button(MouseButton button) const {
  return test(shared_state->buttons_down, static_cast<int>(button));
}
bool Window::mouse_pressed(MouseButton button) const {
  return test(shared_state->buttons_pressed, static_cast<int>(button));
}
bool Window::mouse_released(MouseButton button) const {
  return test(shared_state->buttons_released, static_cast<int>(button));
}

Vec2 Window::mouse() const { return shared_state->mouse; }
Vec2 Window::mouse_delta() const { return shared_state->mouse_delta; }
Vec2 Window::scroll() const { return shared_state->scroll; }

void Window::on_key(std::function<void(const KeyEvent&)> callback) { shared_state->on_key = std::move(callback); }
void Window::on_mouse_button(std::function<void(const MouseButtonEvent&)> callback) {
  shared_state->on_mouse_button = std::move(callback);
}
void Window::on_scroll(std::function<void(Vec2)> callback) { shared_state->on_scroll = std::move(callback); }
void Window::on_resize(std::function<void(Size)> callback) { shared_state->on_resize = std::move(callback); }

GLFWwindow* Window::handle() const { return shared_state->handle; }

} // namespace lab
