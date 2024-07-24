#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include "lab.h"

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

namespace lab {

State state;
State& get_state() { return state; }

void link(Window& wnd, Webgpu& wgpu) { wnd.surface = glfwGetWGPUSurface(wgpu.instance, reinterpret_cast<GLFWwindow*>(wnd.get_handle())); }

void swt(Window& wnd, Webgpu& wgpu) {
  auto dims = wnd.get_dimensions();
  wgpu.configure_surface(wnd.surface, dims.width, dims.height);
}

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
