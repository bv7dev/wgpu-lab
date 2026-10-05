#include <webgpu/webgpu_cpp.h>

#include <extra/lab_state.h>
#include <lab>

#include <GLFW/glfw3.h>

namespace lab {

State state;

bool init_lab() {
  if (!state.lab_init) {
    // initialize glfw
    if (!glfwInit()) {
      std::cerr << "Error: GLFW: Failed to initialize!" << std::endl;
      return false;
    }
    std::cout << "Info: GLFW: Initialized!" << std::endl;

    // GLFW stays initialized until the program exits, see Window::close()
    std::atexit(glfwTerminate);

    // initialize only once
    state.lab_init = true;
  }
  return state.lab_init;
}

// Number of ticks after which all windows are closed, read from the environment
// variable LAB_EXIT_AFTER_FRAMES. Zero means run until the user closes the windows.
// It lets every sample run unattended: `LAB_EXIT_AFTER_FRAMES=120 ./sample_texture`
static long exit_after_frames() {
  static const long frames = [] {
    const char* value = std::getenv("LAB_EXIT_AFTER_FRAMES");
    return value ? std::atol(value) : 0L;
  }();
  return frames;
}

bool tick() {
  glfwPollEvents();

  std::vector<WindowHandle> to_close;
  to_close.reserve(state.window_map.size());

  static long tick_count = 0;
  const bool exit_requested = exit_after_frames() > 0 && ++tick_count > exit_after_frames();
  if (exit_requested) {
    tick_count = 0;
  }

  for (auto [hnd, wnd] : state.window_map) {
    if (exit_requested || glfwWindowShouldClose(hnd)) {
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
