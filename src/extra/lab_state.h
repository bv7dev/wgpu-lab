#ifndef WGPU_LAB_STATE_H
#define WGPU_LAB_STATE_H

#include <extra/lab_objects.h>

#include <chrono>
#include <unordered_map>

// Note: user should not interact with state directly
//       so it's not included in lab.h

namespace lab {

using WindowMap = std::unordered_map<GLFWwindowHandle, WindowHandle>;

extern struct State {
  WindowMap window_map{};
  bool glfw_init = false;
  std::chrono::steady_clock::time_point start_tp = std::chrono::steady_clock::now();
} state;

} // namespace lab

#endif // WGPU_LAB_STATE_H
