#include "lab_window.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <unordered_map>

struct {
  bool glfw_initialized = false;
  std::unordered_map<lab::WindowHandle, lab::Window*> glfw_windows;
} _state;

namespace lab {

bool tick() {
  glfwPollEvents();

  auto& wnds = _state.glfw_windows;
  std::vector<WindowHandle> to_erase;
  to_erase.reserve(wnds.size());

  for (auto [wnd, _] : wnds) {
    if (glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(wnd))) {
      to_erase.push_back(wnd);
    }
  }
  for (auto wnd : to_erase) {
    _state.glfw_windows[wnd]->~Window();
  }

  if (wnds.size() == 0) {
    if (_state.glfw_initialized) {
      glfwTerminate();
      _state.glfw_initialized = false;
      std::cout << "Info: GLFW: Terminated!" << std::endl;
    }
    return false;
  }

  return true;
}

Window::Window(int width, int height, const char* title) {
  if (!_state.glfw_initialized) {
    if (!glfwInit()) {
      std::cerr << "Error: GLFW: Failed to initialize!" << std::endl;
      return;
    }
    std::cout << "Info: GLFW: Initialized!" << std::endl;
    _state.glfw_initialized = true;
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  _handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!_handle) {
    std::cerr << "Error: GLFW: Failed to create Window!" << std::endl;
    return;
  }
  std::cout << "Info: GLFW: Window(" << _handle << ") created!" << std::endl;
  _state.glfw_windows[_handle] = this;
}

void Window::set_key_callback(KeyCallback cb) {
  _key_callback = cb;
  glfwSetKeyCallback(reinterpret_cast<GLFWwindow*>(_handle), [](GLFWwindow* wnd, int key, int scancode, int action, int mod) {
    _state.glfw_windows[wnd]->_key_callback(key, scancode, action, mod);
  });
}

void Window::clear_key_callback() {
  _state.glfw_windows[_handle] = nullptr;
  glfwSetKeyCallback(reinterpret_cast<GLFWwindow*>(_handle), nullptr);
}

bool Window::is_open() { return _handle != nullptr; }

Window::~Window() {
  if (_handle) {
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(_handle));
    _state.glfw_windows.erase(_handle);
    std::cout << "Info: GLFW: Window(" << _handle << ") destroyed!" << std::endl;
    _handle = nullptr;
  }
}

} // namespace lab
