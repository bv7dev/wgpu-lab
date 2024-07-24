#include "lab.h"

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

namespace lab {

State state;
State& get_state() { return state; }

wgpu::Surface prp_surf;
wgpu::Surface orig_surf;

void link(Window& wnd, Webgpu& wgpu) {
  state.wgpu_map[wnd.get_handle()] = reinterpret_cast<Handle>(&wgpu);
  wgpu.surface = glfwGetWGPUSurface(wgpu.instance, reinterpret_cast<GLFWwindow*>(wnd.get_handle()));
  orig_surf = wgpu.surface;
  wgpu.init();
  auto dims = wnd.get_dimensions();
  wgpu.configure_surface(nullptr, dims.width, dims.height);
  wgpu.create_pipeline();
}

void prp(Window& wnd, Webgpu& wgpu) { prp_surf = glfwGetWGPUSurface(wgpu.instance, reinterpret_cast<GLFWwindow*>(wnd.get_handle())); }

void swt(Window& wnd, Webgpu& wgpu) {
  auto dims = wnd.get_dimensions();
  orig_surf = wgpu.configure_surface(prp_surf, dims.width, dims.height);
}

void bkk(Window& wnd, Webgpu& wgpu) {
  auto dims = wnd.get_dimensions();
  prp_surf = wgpu.configure_surface(orig_surf, dims.width, dims.height);
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
