#include "lab_window.h"
#include "lab_state.h"

#include <GLFW/glfw3.h>

#include <iostream>

namespace lab {

Window::Window(int width, int height, const char* title) {
  if (!state.init) {
    if (!glfwInit()) {
      std::cerr << "Error: GLFW: Failed to initialize!" << std::endl;
      return;
    }
    std::cout << "Info: GLFW: Initialized!" << std::endl;
    state.init = true;
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!handle) {
    std::cerr << "Error: GLFW: Failed to create Window!" << std::endl;
    return;
  }
  std::cout << "Info: GLFW: Window(" << glfwGetWindowTitle(reinterpret_cast<GLFWwindow*>(handle)) << " - " << handle << ") created!" << std::endl;
  state.window_map[handle] = this;
}

void Window::set_key_callback(KeyCallback kcb) {
  keycb = kcb;
  glfwSetKeyCallback(reinterpret_cast<GLFWwindow*>(handle), [](GLFWwindow* wnd, int key, int scancode, int action, int mod) {
    reinterpret_cast<Window*>(state.window_map[wnd])->keycb(key, scancode, action, mod);
  });
}

void Window::clear_key_callback() {
  state.window_map[handle] = nullptr;
  glfwSetKeyCallback(reinterpret_cast<GLFWwindow*>(handle), nullptr);
}

int Window::width() const {
  int width;
  glfwGetWindowSize(reinterpret_cast<GLFWwindow*>(handle), &width, nullptr);
  return width;
}

int Window::height() const {
  int height;
  glfwGetWindowSize(reinterpret_cast<GLFWwindow*>(handle), &height, nullptr);
  return height;
}

Handle Window::get_handle() const { return handle; }

bool Window::is_open() const { return handle != nullptr; }

Window::~Window() {
  if (handle) {
    std::cout << "Info: GLFW: Window(" << glfwGetWindowTitle(reinterpret_cast<GLFWwindow*>(handle)) << " - " << handle << ") destroyed!" << std::endl;
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(handle));
    state.window_map.erase(handle);
    handle = nullptr;
    if (state.window_map.size() == 0) {
      if (state.init) {
        glfwTerminate();
        state.init = false;
        std::cout << "Info: GLFW: Terminated!" << std::endl;
      }
    }
  }
}

} // namespace lab
