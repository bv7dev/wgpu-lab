#include "lab_window.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <unordered_map>

struct {
  bool backend_ready = false;
  std::unordered_map<lab::WindowHandle, lab::Window*> window_map;
} _state;

namespace lab {

// Window class ----------------------------------------------------------------
Window::Window(int width, int height, const char* title) {
  if (!_state.backend_ready) {
    if (!glfwInit()) {
      std::cerr << "Error: GLFW: Failed to initialize!" << std::endl;
      return;
    }
    std::cout << "Info: GLFW: Initialized!" << std::endl;
    _state.backend_ready = true;
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  _handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!_handle) {
    std::cerr << "Error: GLFW: Failed to create Window!" << std::endl;
    return;
  }
  std::cout << "Info: GLFW: Window(" << glfwGetWindowTitle(reinterpret_cast<GLFWwindow*>(_handle)) << " - " << _handle << ") created!"
            << std::endl;
  _state.window_map[_handle] = this;
}

void Window::set_key_callback(KeyCallback cb) {
  _key_callback = cb;
  glfwSetKeyCallback(reinterpret_cast<GLFWwindow*>(_handle), [](GLFWwindow* wnd, int key, int scancode, int action, int mod) {
    _state.window_map[wnd]->_key_callback(key, scancode, action, mod);
  });
}

void Window::clear_key_callback() {
  _state.window_map[_handle] = nullptr;
  glfwSetKeyCallback(reinterpret_cast<GLFWwindow*>(_handle), nullptr);
}

Window::Dimensions Window::get_dimensions() const {
  int width, height;
  glfwGetWindowSize(reinterpret_cast<GLFWwindow*>(_handle), &width, &height);
  return {width, height};
}

WindowHandle Window::get_handle() const { return _handle; }

bool Window::is_open() const { return _handle != nullptr; }

Window::~Window() {
  if (_handle) {
    std::cout << "Info: GLFW: Window(" << glfwGetWindowTitle(reinterpret_cast<GLFWwindow*>(_handle)) << " - " << _handle << ") destroyed!"
              << std::endl;
    _state.window_map.erase(_handle);
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(_handle));
    _handle = nullptr;
  }
}

// public functions ------------------------------------------------------------
bool tick() {
  glfwPollEvents();

  auto& wnds = _state.window_map;
  std::vector<WindowHandle> to_erase;
  to_erase.reserve(wnds.size());

  for (auto [wnd, _] : wnds) {
    if (glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(wnd))) {
      to_erase.push_back(wnd);
    }
  }
  for (auto wnd : to_erase) {
    _state.window_map[wnd]->~Window();
  }

  if (wnds.size() == 0) {
    if (_state.backend_ready) {
      glfwTerminate();
      _state.backend_ready = false;
      std::cout << "Info: GLFW: Terminated!" << std::endl;
    }
    return false;
  }

  return true;
}

} // namespace lab
