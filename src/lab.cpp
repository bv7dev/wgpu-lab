#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "lab.h"
#include "lab_state.h"

#include <GLFW/glfw3.h>

namespace lab {

State state;

bool tick() {
  glfwPollEvents();

  std::vector<Handle> to_erase;
  to_erase.reserve(state.window_map.size());

  for (auto [wnd, _] : state.window_map) {
    if (glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(wnd))) {
      to_erase.push_back(wnd);
    }
  }
  for (auto wnd : to_erase) {
    reinterpret_cast<Window*>(state.window_map[wnd])->~Window();
  }

  return state.window_map.size() > 0;
}

} // namespace lab
