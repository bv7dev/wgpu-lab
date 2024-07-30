#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "lab.h"
#include "lab_state.h"

#include <GLFW/glfw3.h>

namespace lab {

State state;

bool init_lab() {
  if (!state.glfw_init) {
    if (!glfwInit()) {
      std::cerr << "Error: GLFW: Failed to initialize!" << std::endl;
      return false;
    }
    std::cout << "Info: GLFW: Initialized!" << std::endl;
    state.glfw_init = true;
    return true;
  }
  //
  // potentially more init stuff in the future
  //
  return false;
}

bool tick() {
  glfwPollEvents();

  std::vector<GLFWwindow*> to_erase;
  to_erase.reserve(state.window_map.size());

  for (auto [wnd, _] : state.window_map) {
    if (glfwWindowShouldClose(wnd)) {
      to_erase.push_back(wnd);
    }
  }
  for (auto wnd : to_erase) {
    state.window_map[wnd]->~Window();
  }

  return state.window_map.size() > 0;
}

bool operator==(const KeyEvent& lhs, const KeyEvent& rhs) {
  return lhs.key == rhs.key && lhs.mod == rhs.mod && lhs.action == rhs.action;
}

} // namespace lab
