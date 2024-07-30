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
  std::cout << "Info: GLFW: Window(" << glfwGetWindowTitle(reinterpret_cast<GLFWwindow*>(handle))
            << " - " << handle << ") created!" << std::endl;
  state.window_map[handle] = this;
}

void Window::set_key_callback(KeyCallback kcb) {
  user_key_callback = kcb;
  glfwSetKeyCallback(reinterpret_cast<GLFWwindow*>(handle), [](GLFWwindow* wnd, int key,
                                                               int scancode, int action, int mod) {
    reinterpret_cast<Window*>(state.window_map[wnd])
        ->user_key_callback({static_cast<KeyCode>(key), static_cast<InputAction>(action),
                             static_cast<ModKey>(mod), scancode});
  });
}

void Window::set_resize_callback(std::function<void(int width, int height)> rcb) {
  user_resize_callback = rcb;
  glfwSetWindowSizeCallback(
      reinterpret_cast<GLFWwindow*>(handle), [](GLFWwindow* wnd, int w, int h) {
        reinterpret_cast<Window*>(state.window_map[wnd])->user_resize_callback(w, h);
      });
}

void Window::clear_key_callback() {
  glfwSetKeyCallback(reinterpret_cast<GLFWwindow*>(handle), nullptr);
  user_key_callback = nullptr;
}

void Window::clear_resize_callback() {
  glfwSetWindowSizeCallback(reinterpret_cast<GLFWwindow*>(handle), nullptr);
  user_resize_callback = nullptr;
}

void Window::set_title(const char* title) {
  glfwSetWindowTitle(reinterpret_cast<GLFWwindow*>(handle), title);
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
    std::cout << "Info: GLFW: Window(" << glfwGetWindowTitle(reinterpret_cast<GLFWwindow*>(handle))
              << " - " << handle << ") destroyed!" << std::endl;
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(handle));
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
