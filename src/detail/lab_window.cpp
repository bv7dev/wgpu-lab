#include <objects/lab_window.h>

#include <extra/lab_enums.h>
#include <extra/lab_state.h>

#include <GLFW/glfw3.h>

#include <iostream>

namespace lab {

bool init_lab();

Window::Window(const char* title, int width, int height) {
  init_lab();
  glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfw_window_handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!glfw_window_handle) {
    std::cerr << "Error: GLFW: Failed to create Window!" << std::endl;
    return;
  }
  std::cout << "Info: GLFW: Window(" << glfwGetWindowTitle(glfw_window_handle) << " - " << glfw_window_handle
            << ") created!" << std::endl;
  state.window_map[glfw_window_handle] = this;
}

void Window::set_key_callback(std::function<void(const KeyEvent&)> kcb) {
  user_key_callback = kcb;
  glfwSetKeyCallback(glfw_window_handle, [](GLFWwindow* wnd, int key, int scancode, int action, int mod) {
    state.window_map[wnd]->user_key_callback(
        {static_cast<KeyCode>(key), static_cast<KeyAction>(action), static_cast<ModKey>(mod), scancode});
  });
}

void Window::set_resize_callback(std::function<void(int width, int height)> rcb) {
  user_resize_callback = rcb;
  glfwSetWindowSizeCallback(glfw_window_handle,
                            [](GLFWwindow* wnd, int w, int h) { state.window_map[wnd]->user_resize_callback(w, h); });
}

void Window::clear_key_callback() {
  glfwSetKeyCallback(glfw_window_handle, nullptr);
  user_key_callback = nullptr;
}

void Window::clear_resize_callback() {
  glfwSetWindowSizeCallback(glfw_window_handle, nullptr);
  user_resize_callback = nullptr;
}

void Window::set_title(const char* title) { glfwSetWindowTitle(glfw_window_handle, title); }

int Window::width() const {
  int width;
  glfwGetWindowSize(glfw_window_handle, &width, nullptr);
  return width;
}

int Window::height() const {
  int height;
  glfwGetWindowSize(glfw_window_handle, nullptr, &height);
  return height;
}

float Window::ratio() const { return (1.f / width()) * height(); }

bool Window::is_open() const { return glfw_window_handle != nullptr; }

void Window::close() {
  if (glfw_window_handle) {
    std::cout << "Info: GLFW: Window(" << glfwGetWindowTitle(glfw_window_handle) << " - " << glfw_window_handle
              << ") destroyed!" << std::endl;
    glfwDestroyWindow(glfw_window_handle);
    state.window_map.erase(glfw_window_handle);
    glfw_window_handle = nullptr;
    if (state.window_map.size() == 0) {
      if (state.lab_init) {
        glfwTerminate();
        state.lab_init = false;
        std::cout << "Info: GLFW: Terminated!" << std::endl;
      }
    }
  }
}
Window::~Window() { close(); }

} // namespace lab
