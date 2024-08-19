#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#include <extra/lab_state.h>
#include <lab>

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

  std::vector<WindowHandle> to_erase;
  to_erase.reserve(state.window_map.size());

  for (auto [hnd, wnd] : state.window_map) {
    if (glfwWindowShouldClose(hnd)) {
      to_erase.push_back(wnd);
    }
  }
  for (auto wnd : to_erase) {
    wnd->close();
  }

  return state.window_map.size() > 0;
}

// todo: figure out what device.tick() actually does,
//       and when it should be called.
bool tick(Webgpu& webgpu) {
  webgpu.device.tick();
  return tick();
}

bool operator==(const KeyEvent& lhs, const KeyEvent& rhs) {
  return lhs.key == rhs.key && lhs.mod == rhs.mod && lhs.action == rhs.action;
}

void reset_time() { state.start_tp = std::chrono::steady_clock::now(); }

// should be precise for around 2500 h program runtime
float elapsed_seconds() {
  return static_cast<float>(
      static_cast<double>((std::chrono::steady_clock::now() - state.start_tp).count()) /
      std::chrono::steady_clock::time_point::period::den);
}

} // namespace lab
