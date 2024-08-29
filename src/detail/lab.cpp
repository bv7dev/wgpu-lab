#include <dawn/webgpu_cpp.h>

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

  std::vector<WindowHandle> to_close;
  to_close.reserve(state.window_map.size());

  for (auto [hnd, wnd] : state.window_map) {
    if (glfwWindowShouldClose(hnd)) {
      to_close.push_back(wnd);
    }
  }
  for (auto wnd : to_close) {
    wnd->close();
  }

  return state.window_map.size() > 0;
}

bool operator==(const KeyEvent& lhs, const KeyEvent& rhs) {
  return lhs.key == rhs.key && lhs.mod == rhs.mod && lhs.action == rhs.action;
}

void restart_timer() { state.start_tp = std::chrono::steady_clock::now(); }
std::chrono::steady_clock::time_point get_timer_start() { return state.start_tp; }

} // namespace lab
