#include "lab_window.h"
#include "lab_enums.h"
#include "lab_state.h"

#include <GLFW/glfw3.h>

#include <iostream>

namespace lab {

bool init_lab();

Window::Window(const char* title, int width, int height) {
  init_lab();
  glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!handle) {
    std::cerr << "Error: GLFW: Failed to create Window!" << std::endl;
    return;
  }
  std::cout << "Info: GLFW: Window(" << glfwGetWindowTitle(handle) << " - " << handle
            << ") created!" << std::endl;
  state.window_map[handle] = this;
}

void Window::set_key_callback(KeyCallback kcb) {
  user_key_callback = kcb;
  glfwSetKeyCallback(handle, [](GLFWwindow* wnd, int key, int scancode, int action, int mod) {
    state.window_map[wnd]->user_key_callback({static_cast<KeyCode>(key),
                                              static_cast<InputType>(action),
                                              static_cast<ModKey>(mod), scancode});
  });
}

void Window::set_resize_callback(std::function<void(int width, int height)> rcb) {
  user_resize_callback = rcb;
  glfwSetWindowSizeCallback(handle, [](GLFWwindow* wnd, int w, int h) {
    state.window_map[wnd]->user_resize_callback(w, h);
  });
}

void Window::clear_key_callback() {
  glfwSetKeyCallback(handle, nullptr);
  user_key_callback = nullptr;
}

void Window::clear_resize_callback() {
  glfwSetWindowSizeCallback(handle, nullptr);
  user_resize_callback = nullptr;
}

void Window::set_title(const char* title) { glfwSetWindowTitle(handle, title); }

int Window::width() const {
  int width;
  glfwGetWindowSize(handle, &width, nullptr);
  return width;
}

int Window::height() const {
  int height;
  glfwGetWindowSize(handle, &height, nullptr);
  return height;
}

GlfwWindowHandle Window::get_handle() const { return handle; }

bool Window::is_open() const { return handle != nullptr; }

Window::~Window() {
  if (handle) {
    std::cout << "Info: GLFW: Window(" << glfwGetWindowTitle(handle) << " - " << handle
              << ") destroyed!" << std::endl;
    glfwDestroyWindow(handle);
    state.window_map.erase(handle);
    handle = nullptr;
    if (state.window_map.size() == 0) {
      if (state.glfw_init) {
        glfwTerminate();
        state.glfw_init = false;
        std::cout << "Info: GLFW: Terminated!" << std::endl;
      }
    }
  }
}

} // namespace lab
