#ifndef WGPU_LAB_STATE_H
#define WGPU_LAB_STATE_H

#include <unordered_map>

// Note: user should not interact with state directly
//       so it's not included in lab.h

struct GLFWwindow;

namespace lab {

using LabWindowMap = std::unordered_map<GLFWwindow*, Window*>;

extern struct State {
  bool glfw_init = false;
  LabWindowMap window_map{};
} state;

} // namespace lab

#endif // WGPU_LAB_STATE_H
